content: drivers.config event_filter.config en_us.chargen special.chargen devices

devices:
	cp $(BASE_DIR)/board/pbxa9/$@ $@

drivers.config event_filter.config:
	cp $(REP_DIR)/recipes/raw/drivers_interactive-pbxa9/$@ $@

en_us.chargen special.chargen:
	cp $(REP_DIR)/src/server/event_filter/$@ $@
