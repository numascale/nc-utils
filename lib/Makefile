TARGETS   = libnumachip_user.so test

CFLAGS    = -O2 -D_GNU_SOURCE -Wall -g

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

libnumachip_user.so: numachip_user.c numachip_user.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ numachip_user.c -lpci -lz

test: test.c numachip_user.h
	$(CC) $(CFLAGS) -o $@ test.c -L. -lnumachip_user
