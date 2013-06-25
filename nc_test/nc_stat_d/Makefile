CC	:= gcc
CFLAGS	:= -O2 -D_GNU_SOURCE -Wall -g -I../../lib
LDFLAGS	:= -L../../lib -lnumachip_user

TARGETS	:= nc_stat_d

#TARGETS += nc_stat_papi_d
#PAPIDIR = /net/numastore/storage/home/av/papi/papi-5.0.0/src
#PAPIINC = $(PAPIDIR)
#LDFLAGS_PTHREAD =  /net/numastore/storage/home/av/papi/papi-5.0.0/src/libpapi.a -lpthread 

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

nc_stat_d: nc_stat_d.c ../../lib/numachip_user.h
	$(CC) $(CFLAGS) -o $@ nc_stat_d.c $(LDFLAGS)

nc_stat_papi_d: nc_stat_d.c ../../lib/numachip_user.h
	$(CC) $(CFLAGS) -I$(PAPIINC) -DWITH_PAPI -o $@ nc_stat_d.c $(LDFLAGS) -L$(PAPIDIR) $(LDFLAGS_PTHREAD)
