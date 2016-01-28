#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# sat_solver files
SRC_CC = inc_sat_solver.cpp

LIBS = stdcxx z3-solver z3-core_tactics z3-aig_tactic z3-bv_tactics z3-arith_tactics z3-sat_tactic gmp

vpath %.cpp $(Z3_DIR)/src/sat/sat_solver
