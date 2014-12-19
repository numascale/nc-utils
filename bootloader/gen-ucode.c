#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>

static void convert_buf_uint32_t(char *src, int max_offset)
{
	char *b;
	int offs = 0;
	printf("    ");

	for (b = strtok(src, " \n");
	     b != NULL && offs < max_offset;
	     b = strtok(NULL, "\n")) {
		if (b[0] == '@') {
			offs = strtol(&b[1], NULL, 16);
			assert(offs < max_offset);
		} else {
			if ((offs % 4) < 3)
				printf("0x%08lx, ", strtol(b, NULL, 16));
			else
				printf("0x%08lx,\n    ", strtol(b, NULL, 16));

			offs++;
		}
	}

	printf("\n");
}

static void convert_buf_uint16_t(char *src, int max_offset)
{
	char *b;
	int offs = 0;
	printf("    ");

	for (b = strtok(src, " \n"); b != NULL && offs < max_offset; b = strtok(NULL, "\n")) {
		if (b[0] == '@') {
			offs = strtol(&b[1], NULL, 16);
			assert(offs < max_offset);
		} else {
			if ((offs % 4) < 3)
				printf("0x%04lx, ", strtol(b, NULL, 16));
			else
				printf("0x%04lx,\n    ", strtol(b, NULL, 16));

			offs++;
		}
	}

	printf("\n");
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("usage: gen-ucode <table> <ucode>\n");
		exit(1);
	}

	{
		int fd = open(argv[1], O_RDONLY);

		if (fd == -1) {
			perror("open");
			exit(1);
		}

		struct stat stat;
		assert(fstat(fd, &stat) == 0);
		char *data = (char *)malloc(stat.st_size);

		if (read(fd, data, stat.st_size) < stat.st_size) {
			perror("read");
			exit(1);
		}

		assert(close(fd) == 0);
		printf("static const uint16_t numachip_mseq_table[] = {\n");
		convert_buf_uint16_t(data, stat.st_size / sizeof(data[0]));
		printf("};\n");
		free(data);
	}

	{
		int fd = open(argv[2], O_RDONLY);

		if (fd == -1) {
			perror("open");
			exit(1);
		}

		struct stat stat;
		assert(fstat(fd, &stat) == 0);
		char *data = (char *)malloc(stat.st_size);
		assert(data);

		if (read(fd, data, stat.st_size) < 1) {
			perror("read");
			free(data);
			exit(1);
		}

		assert(close(fd) == 0);
		printf("static uint32_t numachip_mseq_ucode[] = {\n");
		convert_buf_uint32_t(data, stat.st_size / sizeof(data[0]));
		printf("};\n");
		free(data);
	}

	return 0;
}
