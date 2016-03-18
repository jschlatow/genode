#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# smt_tactic files
SRC_CC = unit_subsumption_tactic.cpp \
	smt_tactic.cpp \
	ctx_solver_simplify_tactic.cpp

LIBS = stdcxx z3-smt

vpath %.cpp $(Z3_DIR)/src/smt/tactic
