TARGETS	:= numaplace libnumaplace.so numaplace-test
CFLAGS	:= -std=gnu99 -Wall -Wextra -Wshadow -O3 -ggdb3 -D_GNU_SOURCE -Wno-unused-parameter -ftls-model=initial-exec -fomit-frame-pointer
CXXFLAGS := -Wall -Wextra -O3 -g

.PHONY: all
all: $(TARGETS) numaplace.pdf

.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o version.h numaplace.pdf

.PHONY: check
check:
	cppcheck --enable=all --inconclusive *.c *.c *.h

.PHONY: prepare
prepare:
	sudo apt-get install latex-beamer

.PHONY: upload
upload: all
	rsync numaplace libnumaplace.so resources:resources/numaplace
	rsync numaplace libnumaplace.so demo-gw:/opt/bin
	rsync numaplace libnumaplace.so build:bin
	rsync numaplace.pdf resources:resources/numaplace

version.h: Makefile numaplace.c libnumaplace.c utils.h
	@echo \#define VER \"`git describe --always --dirty=+`\" >version.h

#numaplace: numaplace.c libnumaplace.c utils.h version.h
#	$(CC) $(CFLAGS) -DPARENT -o $@ $< ncalloc.c libnumaplace.c -lpthread -ldl -lnuma -lrt

numaplace: numaplace.cc
	$(CXX) $(CXXFLAGS) -o $@ $<

numaplace.pdf: numaplace.tex
	pdflatex -halt-on-error -file-line-error-style numaplace.tex
	rm numaplace.log numaplace.nav numaplace.out numaplace.snm numaplace.toc numaplace.vrb

numaplace-test: numaplace-test.c utils.h
	$(CC) $(CFLAGS) -o $@ $< -lpthread

libnumaplace.so: libnumaplace.c ncalloc.c utils.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ ncalloc.c $< -ldl -lnuma -lrt
