CC      := gcc
CFLAGS  := -O2 -D_GNU_SOURCE -Wall -g -I../../lib
LDFLAGS := -L../../lib -lnumachip_user

TARGETS := nc_log_d

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

nc_log_d: nc_log_d.c ../../lib/numachip_user.h
	$(CC) $(CFLAGS) -o $@ nc_log_d.c $(LDFLAGS)
