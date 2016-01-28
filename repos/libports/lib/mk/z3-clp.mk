#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# clp files
SRC_CC = clp_context.cpp

LIBS = stdcxx z3-muz z3-transforms gmp

vpath %.cpp $(Z3_DIR)/src/muz/clp
