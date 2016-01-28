#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# interval files
SRC_CC = interval_mpq.cpp

LIBS = stdcxx z3-util gmp

vpath %.cpp $(Z3_DIR)/src/math/interval
