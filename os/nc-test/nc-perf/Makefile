TARGETS   = nc_perf 

CFLAGS    = -O2 -D_GNU_SOURCE -Wall -g

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

nc_perf: nc_perf.c ../../lib/pcounter_test.c ../../lib/numachip_user.h
	$(CC) $(CFLAGS) -I ../../lib  -o $@ nc_perf.c ../../lib/pcounter_test.c -L../../lib/ -lnumachip_user 
