#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# fuzzing files
SRC_CC = expr_rand.cpp \
	expr_delta.cpp

LIBS = stdcxx z3-ast

vpath %.cpp $(Z3_DIR)/src/test/fuzzing
