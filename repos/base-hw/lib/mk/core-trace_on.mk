#
# \brief  Build config for parts of core that depend on tracing status
# \author Johannes Schlatow
# \date   2016-07-21
#

SRC_CC += kernel/trace.cc

INC_DIR += $(REP_DIR)/src/include
INC_DIR += $(REP_DIR)/src/core/include
INC_DIR += $(BASE_DIR)/src/include
INC_DIR += $(BASE_DIR)/src/core/include

vpath % $(REP_DIR)/src/core
