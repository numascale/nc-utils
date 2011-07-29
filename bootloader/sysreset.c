#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <console.h>
#include <com32.h>
#include <inttypes.h>
#include <sys/io.h>


int main(void)
{
    openconsole(&dev_rawcon_r, &dev_stdcon_w);

    printf("Resetting system...\n");

    outb(0xa, 0xcf9);
    outb(0xe, 0xcf9);

    return -1;
}
