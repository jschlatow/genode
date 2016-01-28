#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# bmc files
SRC_CC = dl_bmc_engine.cpp

LIBS = stdcxx z3-muz z3-transforms gmp

vpath %.cpp $(Z3_DIR)/src/muz/bmc
