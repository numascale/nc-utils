TARGETS   = nc_stat_d

CFLAGS    = -O2 -D_GNU_SOURCE -Wall -g

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

nc_stat_d: nc_stat_d.c ../../lib/numachip_user.h
	$(CC) $(CFLAGS) -I ../../lib  -o $@ nc_stat_d.c -L../../lib/ -lnumachip_user 
