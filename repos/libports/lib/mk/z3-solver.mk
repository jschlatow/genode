#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# solver files
SRC_CC = tactic2solver.cpp \
	solver_na2as.cpp \
	solver.cpp \
	combined_solver.cpp \
	check_sat_result.cpp

LIBS = stdcxx z3-model z3-tactic gmp

vpath %.cpp $(Z3_DIR)/src/solver
