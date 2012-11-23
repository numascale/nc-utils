CC	:= gcc

CFLAGS	:= -O2 -D_GNU_SOURCE -Wall -g

TARGETS	:= libnumachip_user.so libnumachip_memops.so

USERSRC	:= init.c device.c pcounter.c pcounter_test.c sysfs.c parse_json.c ../../bootloader/json-1.5/src/json.c

# Use Open64 compiler to compile the memops library (due to AVX instruction availability).
MEMCC	:= opencc

MEMSRC	:= memops.c

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

libnumachip_user.so: $(USERSRC) numachip_lib.h numachip_user.h
	$(CC) $(CFLAGS) -I ../../bootloader/ -I ../../bootloader/json-1.5/src -fPIC -shared -o $@ $(USERSRC) 

libnumachip_memops.so: $(MEMSRC) numachip_memops.h
	$(MEMCC) $(CFLAGS) -fPIC -shared -o $@ $(MEMSRC) 

