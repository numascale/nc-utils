TARGETS	:= numaplace libnumaplace.so numaplace-test
CFLAGS	:= -Wall -Wextra -Wshadow -O3 -g

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o

check:
	cppcheck --enable=all *.c *.cc *.h

numaplace: numaplace.cc utils.h
	$(CXX) $(CFLAGS) -o $@ $<

numaplace-test: numaplace-test.c utils.h
	$(CXX) $(CFLAGS) -o $@ $< -lpthread

libnumaplace.so: libnumaplace.cc utils.h
	$(CXX) $(CFLAGS) -fPIC -shared -o $@ $< -lrt -ldl -lnuma
