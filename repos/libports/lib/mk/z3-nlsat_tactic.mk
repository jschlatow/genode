#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# nlsat_tactic files
SRC_CC = qfnra_nlsat_tactic.cpp \
	nlsat_tactic.cpp \
	goal2nlsat.cpp

LIBS = stdcxx z3-nlsat z3-sat_tactic z3-arith_tactics

vpath %.cpp $(Z3_DIR)/src/nlsat/tactic
