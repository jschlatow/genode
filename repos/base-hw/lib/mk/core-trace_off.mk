#
# \brief  Build config for parts of core that depend on tracing status
# \author Johannes Schlatow
# \date   2016-07-21
#

SRC_CC += kernel/no_trace.cc

INC_DIR +=  $(REP_DIR)/src/core/include

vpath %  $(REP_DIR)/src/core
