TARGETS   = libnumachip_user.so test

CFLAGS    = -O2 -D_GNU_SOURCE -Wall -g

LIBSRC    = init.c device.c sysfs.c

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

libnumachip_user.so: $(LIBSRC) numachip_lib.h numachip_user.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $(LIBSRC)

test: test.c numachip_user.h
	$(CC) $(CFLAGS) -o $@ test.c -L. -lnumachip_user
