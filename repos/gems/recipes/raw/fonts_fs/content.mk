PORT_DIR := $(call port_dir,$(GENODE_DIR)/repos/libports/ports/ttf-bitstream-vera)

content:
	cp $(PORT_DIR)/ttf/bitstream-vera/COPYRIGHT.TXT LICENSE.bitstream-vera
	cp $(PORT_DIR)/ttf/bitstream-vera/Vera.ttf ./default.ttf
	cp $(PORT_DIR)/ttf/bitstream-vera/VeraMono.ttf ./monospace.ttf
