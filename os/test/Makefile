SUBDIRS = nc-stat  nc-stat_d

.PHONY: subdirs $(SUBDIRS)

.PHONY: all
all: subdirs $(SUBDIRS)

.PHONY: clean
clean:
	(cd nc-stat && make clean)
	(cd nc-stat_d && make clean)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
