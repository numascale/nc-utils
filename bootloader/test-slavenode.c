/*
 * Copyright (C) 2008-2012 Numascale AS, support@numascale.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE 1

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sched.h>
#include <signal.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dnc-regs.h"
#include "dnc-access.h"
#include "dnc-route.h"
#include "dnc-fabric.h"
#include "dnc-config.h"
#include "dnc-commonlib.h"

int ht_testmode = 0;

static void sighandler(int sig)
{
	printf("Received signal %d. aborting!\n", sig);
	dnc_write_csr(0xfff0, H2S_CSR_G0_RAW_CONTROL, 0x1000); /* Reset RAW engine */
	exit(-1);
}

int read_config_file(char *file_name)
{
	int fd, len;
	char buf[16 * 1024];
	fd = open(file_name, O_RDONLY);

	if (fd < 0) {
		fprintf(stderr, "Unable to read <%s>.\n", file_name);
		return -1;
	}

	len = read(fd, buf, sizeof(buf));
	close(fd);

	if (len <= 0) {
		fprintf(stderr, "No config file contents?\n");
		return -1;
	}

	buf[len] = '\0';

	if (!parse_config_file(buf)) {
		printf("Error reading config file!\n");
		return -1;
	}

	return 0;
}

void test_route(uint8_t bxbarid, uint16_t dest);


int udp_open(void)
{
	int sock;
	int val;
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sock < 0)
		return sock;

	val = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &val, sizeof(val));
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char *) &val, sizeof(val));
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port        = htons(MSG_PORT);

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == 0)
		return sock;
	else
		return -1;
}

void udp_broadcast_state(int sock, void *buf, int len)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = INADDR_BROADCAST;
	addr.sin_port        = htons(MSG_PORT);

	if (sendto(sock, buf, len, 0,
	           (struct sockaddr *) &addr, sizeof(addr)) < len) {
		fprintf(stderr, "Error calling sendto, udp packet not sent!\n");
	}
}

int udp_read_state(int sock, void *buf, int len)
{
	struct sockaddr_in addr;
	socklen_t slen;
	int i;
	char b[1024];
	memset(&addr, 0, sizeof(struct sockaddr_in));
	slen = sizeof(addr);
	i = recvfrom(sock, &b, sizeof(b), MSG_DONTWAIT,
	             (struct sockaddr *) &addr, &slen);

	if (i > 0) printf("Got UDP packet from %s port %d, len = %d\n",
		                  inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), i);

	if ((i > 0) && (addr.sin_port == htons(MSG_PORT)) && (i <= len)) {
		memcpy(buf, &b, i);
		return i;
	}

	return 0;
}

static char *_concat_argv(int argc, char **argv)
{
	int i, n;
	char *res, *cmdline = NULL;

	for (i = 1, n = argc - 1; n > 0; n--, i++) {
		res = realloc(cmdline, (cmdline ? strlen(cmdline) : 0) + strlen(argv[i]) + 2);

		if (!res) {
			free(cmdline);
			return NULL;
		}

		cmdline = res;

		if (i > 1) strcat(cmdline, " ");
		else cmdline[0] = '\0';

		strcat(cmdline, argv[i]);
	}

	return cmdline;
}

bool dnc_asic_mode;
uint32_t dnc_chip_rev;

int main(int argc, char **argv)
{
	int cpu_fam  = -1;
	cpu_set_t cset;
	uint32_t val, uuid = 40;
	char type[16];
	struct node_info *info;
	struct part_info *part;
	char *cmdline = NULL;
	(void) signal(SIGINT, sighandler);
	/* Bind to core 0 */
	CPU_ZERO(&cset);
	CPU_SET(0, &cset);

	if (sched_setaffinity(0, sizeof(cset), &cset) != 0) {
		fprintf(stderr, "Unable to bind to core 0.\n");
		return -1;
	}

	asm volatile("mov $1, %%eax; cpuid" : "=a"(val) :: "ebx", "ecx", "edx");
	cpu_fam = (val >> 8) & 0xf;

	if (cpu_fam == 0xf)
		cpu_fam += (val >> 20) & 0xf;

	if (cpu_fam <= 0xf) {
		fprintf(stderr, "*** Unsupported CPU family %d\n", cpu_fam);
		return -1;
	}

	cmdline = _concat_argv(argc, argv);

	if (dnc_init_bootloader(&uuid, &dnc_chip_rev, type, &dnc_asic_mode, cmdline) < 0)
		return -1;

	info = get_node_config(uuid);

	if (!info)
		return -1;

	part = get_partition_config(info->partition);

	if (!part)
		return -1;

	wait_for_master(info, part);
	udelay(5000);

	if (dnc_init_caches() < 0)
		return -1;

	/* Set G3x02c FAB_CONTROL bit 30 */
	dnc_write_csr(0xfff0, H2S_CSR_G3_FAB_CONTROL, 1 << 30);
	printf("Numascale NumaChip awaiting fabric set-up by master node...\n");

	while (1) {
		uint32_t val;
		val = dnc_read_csr(0xfff0, H2S_CSR_G3_FAB_CONTROL);

		if ((val & (1 << 31))) {
			printf("Go-ahead seen, jumping to trampoline...\n");
			dnc_write_csr(0xfff0, H2S_CSR_G3_FAB_CONTROL, val & ~(1 << 31));
			break;
		}

		dnc_check_fabric(info);
		udelay(1000);
	}

	return 0;
}
