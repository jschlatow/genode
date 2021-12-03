content: src/app/netperf LICENSE

NETPERF_CONTRIB_DIR := $(call port_dir,$(REP_DIR)/ports/netperf)/src/app/netperf

src/app/netperf:
	mkdir -p $@
	cp -r $(NETPERF_CONTRIB_DIR)/* $@
	$(mirror_from_rep_dir)

LICENSE:
	cp $(NETPERF_CONTRIB_DIR)/COPYING $@

