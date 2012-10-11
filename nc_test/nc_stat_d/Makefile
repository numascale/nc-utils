TARGETS   = nc_stat_d nc_stat_papi_d
PAPIDIR = /net/numastore/storage/home/av/papi/papi-5.0.0/src
CFLAGS    += -O2 -D_GNU_SOURCE 	-Wall -g
PAPIINC = $(PAPIDIR)
#CC = gcc
CFLAGS += -I$(PAPIINC)
#OS = $(shell uname)
LDFLAGS_PTHREAD =  /net/numastore/storage/home/av/papi/papi-5.0.0/src/libpapi.a -lpthread

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

nc_stat_papi_d: nc_stat_d.c ../../lib/numachip_user.h
	$(CC) $(CFLAGS) -DWITH_PAPI -I ../../lib -o $@ nc_stat_d.c -L../../lib/ -L$(PAPIDIR) $(LDFLAGS_PTHREAD) -lnumachip_user
nc_stat_d: nc_stat_d.c ../../lib/numachip_user.h
	$(CC) $(CFLAGS) -I ../../lib  -o $@ nc_stat_d.c -L../../lib/ -lnumachip_user
