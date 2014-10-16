TARGETS	:= place libplace.so place-test
CFLAGS	:= -Wall -Wextra -Wshadow -O0 -g

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

check:
	cppcheck --enable=all *.c *.cc *.h

place: place.cc utils.h
	$(CXX) $(CFLAGS) -o $@ $<

place-test: place-test.c utils.h
	$(CXX) $(CFLAGS) -o $@ $< -lpthread

libplace.so: libplace.cc utils.h
	$(CXX) $(CFLAGS) -fPIC -shared -o $@ $< -lrt -ldl
