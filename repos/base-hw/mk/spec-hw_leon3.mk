#
# \brief  Build configurations specific to base-hw and LEON3
# \author Norman Feske
# \date   2013-04-05
#

# denote wich specs are also fullfilled by this spec
SPECS += hw platform_leon3

# configure multiprocessor mode
PROCESSORS = 1

# set address where to link the text segment at
# FIXME LD_TEXT_ADDR
LD_TEXT_ADDR ?= 0x800000

# include implied specs
include $(call select_from_repositories,mk/spec-hw.mk)
include $(call select_from_repositories,mk/spec-platform_leon3.mk)
