SUBDIRS = lib nc-test #examples

.PHONY: subdirs $(SUBDIRS)

.PHONY: all
all: subdirs $(SUBDIRS)

.PHONY: clean
clean:
	for i in $(SUBDIRS); do \
		(cd $$i && make clean) \
        done

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
test: lib
examples: lib
