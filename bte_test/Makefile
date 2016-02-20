CC     = gcc
CFLAGS = -O2 -std=gnu99 -Wall -I../lib -fno-strict-aliasing

.PHONY: all
all: bte_test bte_perf

bte_test: bte_test.c ../lib/libncbte.h
	$(CC) $(CFLAGS) -o $@ bte_test.c -L../lib -lncbte -lncutils_lock -lnuma -lpci

bte_perf: bte_perf.c ../lib/libncbte.h
	$(CC) $(CFLAGS) -o $@ bte_perf.c ../lib/memops.c -L../lib -lncbte -lncutils_lock -lnuma -lpci

.PHONY: clean
clean:
	rm -f bte_test bte_perf *.o *.~*
