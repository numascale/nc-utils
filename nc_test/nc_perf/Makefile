CC      := gcc
CFLAGS  := -O2 -D_GNU_SOURCE -Wall -g -I../../lib -I../../../legacy-firmware/interface
LDFLAGS := -L../../lib -lnumachip_user

TARGETS := nc_perf

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

nc_perf: nc_perf.c ../../lib/pcounter_test.c ../../lib/numachip_user.h
	$(CC) $(CFLAGS) -o $@ nc_perf.c ../../lib/pcounter_test.c $(LDFLAGS)
