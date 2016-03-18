#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# subpaving_tactic files
SRC_CC = subpaving_tactic.cpp \
	expr2subpaving.cpp

LIBS = stdcxx z3-core_tactics z3-subpaving

vpath %.cpp $(Z3_DIR)/src/math/subpaving/tactic
