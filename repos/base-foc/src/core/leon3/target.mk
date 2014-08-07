include $(PRG_DIR)/../target.inc

# FIXME Sparc: LD_TEXT_ADDR
LD_TEXT_ADDR = 0x00100000

REQUIRES += sparc foc_leon3
SRC_CC   += sparc/platform_sparc.cc
INC_DIR  += $(REP_DIR)/src/core/include/sparc

vpath platform_services.cc $(GEN_CORE_DIR)
