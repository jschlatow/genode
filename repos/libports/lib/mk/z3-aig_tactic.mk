#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# aig_tactic files
SRC_CC = aig_tactic.cpp \
	aig.cpp

LIBS = stdcxx z3-tactic gmp

vpath %.cpp $(Z3_DIR)/src/tactic/aig
