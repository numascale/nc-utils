CC     = gcc
CFLAGS = -O2 -Wall -I../lib -fno-strict-aliasing -fopenmp

lock_test: lock_test.c ../lib/ncutils_lock.h ../lib/ncutils_atomic.h
	$(CC) $(CFLAGS) -o $@ lock_test.c -L../lib -lncutils_lock

.PHONY: clean
clean:
	rm -f lock_test *.o *.~*
