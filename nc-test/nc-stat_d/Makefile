TARGETS   = cache_stat_d

CFLAGS    = -O2 -D_GNU_SOURCE -Wall -g

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

cache_stat_d: cache_stat_d.c ../../lib/numachip_user.h
	$(CC) $(CFLAGS) -I ../../lib  -o $@ cache_stat_d.c -L../../lib/ -lnumachip_user 
