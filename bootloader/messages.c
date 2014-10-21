#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "dnc-bootloader.h"
#include "dnc-commonlib.h"
#undef assert

#include <assert.h>


int main(void)
{
	int s = socket(PF_INET, SOCK_DGRAM, 0);
	assert(s != -1);

	const struct sockaddr_in addr = {AF_INET, ntohs(MSG_PORT), INADDR_ANY, 0};
	assert(bind(s, (struct sockaddr *)&addr, sizeof(addr)) != -1);

	int val = 1;
	assert(setsockopt(s, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val)) != -1);

	char pkt[1540];
	struct state_bcast *msg = (struct state_bcast *)pkt;
	const char *str = pkt + sizeof(struct state_bcast);
	const char *states[] = {NODE_SYNC_STATES(ENUM_NAMES)};
	struct sockaddr_in src;
	socklen_t srclen;

	printf("Waiting for network messages\n");
	while (1) {
		srclen = sizeof(src);
		val = recvfrom(s, &pkt, sizeof(pkt), 0, (struct sockaddr *)&src, &srclen);
		assert(val != -1);

		if (val < (int)sizeof(struct state_bcast)) {
			fprintf(stderr, "short message received of %u bytes\n", val);
			continue;
		}

		if (msg->sig != UDP_SIG) {
			fprintf(stderr, "invalid signature 0x%08x\n", msg->sig);
			continue;
		}

		struct hostent *ent = gethostbyaddr(&src, srclen, AF_INET);

		printf("%03x/%s: IP %s, UUID %08x, TID %u, state %s\n", msg->sci,
		  ent ? ent->h_name : "(unknown)", inet_ntoa(src.sin_addr),
		  msg->uuid, msg->tid, states[msg->state]);

		if (msg->state == RSP_ERROR)
			printf(" -> %s", str);
	}
}
