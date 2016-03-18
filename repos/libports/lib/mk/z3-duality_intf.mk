#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# duality_intf files
SRC_CC = duality_dl_interface.cpp

LIBS = stdcxx z3-muz z3-transforms z3-duality

vpath %.cpp $(Z3_DIR)/src/muz/duality
