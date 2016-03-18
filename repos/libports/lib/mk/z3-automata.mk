#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# automata files
SRC_CC = automaton.cpp

LIBS = stdcxx z3-util

vpath %.cpp $(Z3_DIR)/src/math/automata
