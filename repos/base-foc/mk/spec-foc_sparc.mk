#
# Specifics for Fiasco.OC on Sparc
#

SPECS += foc

#
# Linker options that are specific for Sparc
#
LD_TEXT_ADDR ?= 0x40800000

#
# Sparc-specific L4/sys headers
#
L4_INC_DIR  = $(BUILD_BASE_DIR)/include/sparc
L4F_INC_DIR = $(BUILD_BASE_DIR)/include/sparc/l4f

#
# Support for Fiasco.OC's Sparc-specific atomic functions
#
REP_INC_DIR += include/sparc

#
# Defines for L4/sys headers
#
CC_OPT += -DCONFIG_L4_CALL_SYSCALLS -DARCH_sparc

#
# Architecture-specific L4sys header files
#
L4_INC_TARGETS = sparc/l4/sys \
                 sparc/l4f/l4/sys \
                 sparc/l4/vcpu

include $(call select_from_repositories,mk/spec-foc.mk)

INC_DIR += $(L4F_INC_DIR) $(L4_INC_DIR)
