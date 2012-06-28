TARGETS   = libnumachip_user.so test cache_stat_d

CFLAGS    = -O2 -D_GNU_SOURCE -Wall -g

LIBSRC    = init.c device.c pcounter.c pcounter_test.c sysfs.c memops.c parse_json.c ../../bootloader/json-1.4/src/json.c

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

libnumachip_user.so: $(LIBSRC) numachip_lib.h numachip_user.h
	$(CC) $(CFLAGS) -I ../../bootloader/ -I ../bootloader/json-1.4 -fPIC -shared -o $@ $(LIBSRC) 

cache_stat_d: cache_stat_d.c numachip_user.h
	$(CC) $(CFLAGS) -I ../../bootloader/  -o $@ cache_stat_d.c -L. -lnumachip_user 

test: test.c numachip_user.h
	$(CC) $(CFLAGS) -I ../../bootloader/  -o $@ test.c -L. -lnumachip_user 
