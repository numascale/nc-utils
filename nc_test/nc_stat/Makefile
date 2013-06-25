CC      := gcc
CFLAGS  := -O2 -D_GNU_SOURCE -Wall -g -I../../lib
LDFLAGS := -L../../lib -lnumachip_user

TARGETS := test

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

test: test.c ../../lib/numachip_user.h
	$(CC) $(CFLAGS) -o $@ test.c $(LDFLAGS)
