TARGETS	:= numaplace libnumaplace.so numaplace-test
CFLAGS	:= -std=gnu11 -Wall -Wextra -Wshadow -O3 -g -D_GNU_SOURCE

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

check:
	cppcheck --enable=all --inconclusive *.c *.c *.h

upload: all
	rsync $(TARGETS) resources:resources/numaplace
	rsync $(TARGETS) nc-demo:/opt/bin
	rsync $(TARGETS) build:common

numaplace: numaplace.c libnumaplace.c utils.h
	$(CC) $(CFLAGS) -o $@ $< libnumaplace.c -lpthread -ldl -lnuma

numaplace-test: numaplace-test.c utils.h
	$(CC) $(CFLAGS) -o $@ $< -lpthread

libnumaplace.so: libnumaplace.c utils.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $< -ldl -lnuma
