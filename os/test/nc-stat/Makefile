TARGETS   = test

CFLAGS    = -O2 -D_GNU_SOURCE -Wall -g

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

test: test.c ../../lib/numachip_user.h
	$(CC) $(CFLAGS) -I ../../lib  -o $@ test.c -L../../lib/ -lnumachip_user 
