#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# portfolio files
SRC_CC = smt_strategic_solver.cpp \
	default_tactic.cpp

LIBS = stdcxx z3-smtlogic_tactics z3-sat_solver z3-ufbv_tactic z3-fpa_tactics z3-aig_tactic z3-fp z3-qe z3-sls_tactic z3-subpaving_tactic gmp

vpath %.cpp $(Z3_DIR)/src/tactic/portfolio
