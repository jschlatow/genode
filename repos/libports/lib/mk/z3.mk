#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# util files
#SRC_CC   = nic.cc printf.cc sys_arch.cc

LIBS     = stdcxx z3-api z3-sat z3-extra_cmds

#vpath %.cc $(REP_DIR)/src/lib/lwip/platform
#vpath %.cc  $(Z3_DIR)/src/util

SHARED_LIB = yes
