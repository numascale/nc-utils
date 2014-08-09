TARGETS	:= place libplace.so

CC	:= g++
CFLAGS	:= -Wall -Wextra -Wshadow -O3 -g

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

place: place.cc utils.h
	$(CC) $(CFLAGS) -o $@ $<

libplace.so: libplace.cc utils.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<
