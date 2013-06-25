SUBDIRS = nc_log_d nc_perf nc_stat nc_stat_d

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
