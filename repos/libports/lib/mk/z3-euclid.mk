#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# euclid files
SRC_CC = euclidean_solver.cpp

LIBS = stdcxx z3-util gmp

vpath %.cpp $(Z3_DIR)/src/math/euclid
