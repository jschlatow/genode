#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# duality files
SRC_CC = duality_wrapper.cpp \
	duality_solver.cpp \
	duality_rpfp.cpp \
	duality_profiling.cpp

LIBS = stdcxx z3-smt z3-interp z3-qe gmp

vpath %.cpp $(Z3_DIR)/src/duality
