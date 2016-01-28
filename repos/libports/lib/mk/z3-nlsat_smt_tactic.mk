#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# nlsat_smt_tactic files
SRC_CC = nl_purify_tactic.cpp

LIBS = stdcxx z3-nlsat_tactic z3-smt_tactic gmp

vpath %.cpp $(Z3_DIR)/src/tactic/nlsat_smt
