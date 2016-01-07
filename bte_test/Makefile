CC     = gcc
CFLAGS = -O2 -Wall -I../lib -fno-strict-aliasing

bte_test: bte_test.c ../lib/libncbte.h
	$(CC) $(CFLAGS) -o $@ bte_test.c -L../lib -lncbte -lnuma

.PHONY: clean
clean:
	rm -f bte_test *.o *.~*
