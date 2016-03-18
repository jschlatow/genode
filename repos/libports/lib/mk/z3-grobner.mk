#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# grobner files
SRC_CC = grobner.cpp

LIBS = stdcxx z3-ast

vpath %.cpp $(Z3_DIR)/src/math/grobner
