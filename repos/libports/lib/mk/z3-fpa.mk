#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# fpa files
SRC_CC = fpa2bv_converter.cpp

LIBS = stdcxx z3-ast z3-util z3-simplifier

vpath %.cpp $(Z3_DIR)/src/ast/fpa
