CC	:= gcc
CFLAGS	:= -O2 -D_GNU_SOURCE -Wall -g -I../../legacy-firmware/interface

AR	:= ar
ARFLAGS := cr

TARGETS	:= libnumachip_user.so libnumachip_memops.so libncutils_lock.a libncbte.a

USERSRC	:= init.c device.c pcounter.c pcounter_test.c

MEMSRC	:= memops.c

LOCKOBJ := tktqlock.o

BTEOBJ	:= libncbte.o

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

libnumachip_user.so: $(USERSRC) numachip_lib.h numachip_user.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $(USERSRC)

libnumachip_memops.so: $(MEMSRC) numachip_memops.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $(MEMSRC)

libncutils_lock.a: tktqlock.c ncutils_lock.h ncutils_atomic.h
	$(CC) $(CFLAGS) -c $< -o $(LOCKOBJ)
	$(AR) $(ARFLAGS) $@ $(LOCKOBJ)

libncbte.a: libncbte.c libncbte.h
	$(CC) $(CFLAGS) -c $< -o $(BTEOBJ)
	$(AR) $(ARFLAGS) $@ $(BTEOBJ)
