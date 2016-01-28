#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# hilbert files
SRC_CC = hilbert_basis.cpp

LIBS = stdcxx z3-util gmp

vpath %.cpp $(Z3_DIR)/src/math/hilbert
