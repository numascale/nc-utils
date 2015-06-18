TARGETS	:= numaplace libnumaplace.so numaplace-test
CFLAGS	:= -std=gnu99 -Wall -Wextra -Wshadow -O3 -g -D_GNU_SOURCE -Wno-unused-parameter -ftls-model=initial-exec -fomit-frame-pointer

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

check:
	cppcheck --enable=all --inconclusive *.c *.c *.h

upload: all
	rsync numaplace libnumaplace.so resources:resources/numaplace
	rsync numaplace libnumaplace.so nc-demo:/opt/bin
	rsync numaplace libnumaplace.so build:common

numaplace: numaplace.c libnumaplace.c utils.h
	$(CC) $(CFLAGS) -DPARENT -o $@ $< ncalloc.c libnumaplace.c -lpthread -ldl -lnuma

numaplace-test: numaplace-test.c utils.h
	$(CC) $(CFLAGS) -o $@ $< -lpthread

libnumaplace.so: libnumaplace.c ncalloc.c utils.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ ncalloc.c $< -ldl -lnuma
