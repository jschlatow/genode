#
# \brief  Build config for Genodes core process
# \author Johannes Schlatow
# \date   2014-12-15
#

INC_DIR += $(REP_DIR)/src/core/include/spec/parallella

# include less specific configuration
include $(REP_DIR)/lib/mk/zynq/core.inc
