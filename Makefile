SUBDIRS = lib test #examples

.PHONY: subdirs $(SUBDIRS)

.PHONY: all
all: subdirs $(SUBDIRS)

.PHONY: clean
clean:
#	(cd examples && make clean)
	(cd test && make clean)
	(cd lib && make clean)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
test: lib
examples: lib
