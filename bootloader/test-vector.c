#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "dnc-aml.h"
#include "dnc-acpi.h"

void dump(Vector<uint64_t> &vector)
{
	printf("contents:\n");
	for (uint64_t *pos = vector.elements; pos < vector.limit; pos++)
		printf("%llu\n", *pos);
	printf("\n");
}

int main(void)
{
	Vector<uint64_t> vector;

	dump(vector);
	for (uint64_t i = 0; i < 1000; i++)
		vector.add(i);
	for (uint64_t i = 0; i < 999; i++)
		vector.del(vector.used - 1);
	dump(vector);

	return 0;
}

