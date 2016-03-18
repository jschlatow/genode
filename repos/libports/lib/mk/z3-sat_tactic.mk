#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# sat_tactic files
SRC_CC = sat_tactic.cpp \
	goal2sat.cpp \
	atom2bool_var.cpp

LIBS = stdcxx z3-tactic z3-sat

vpath %.cpp $(Z3_DIR)/src/sat/tactic
