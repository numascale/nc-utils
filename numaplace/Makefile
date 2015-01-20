TARGETS	:= numaplace libnumaplace.so numaplace-test
CFLAGS	:= -std=gnu11 -Wall -Wextra -Wshadow -O3 -g -D_GNU_SOURCE

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

check:
	cppcheck --enable=all --inconclusive *.c *.c *.h

numaplace: numaplace.c utils.h
	$(CC) $(CFLAGS) -o $@ $<

numaplace-test: numaplace-test.c utils.h
	$(CC) $(CFLAGS) -o $@ $< -lpthread

libnumaplace.so: libnumaplace.c utils.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $< -ldl -lnuma
