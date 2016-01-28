#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# pattern files
SRC_CC = pattern_inference_params.cpp \
	pattern_inference.cpp \
	expr_pattern_match.cpp

LIBS = stdcxx z3-normal_forms z3-smt2parser z3-simplifier gmp

vpath %.cpp $(Z3_DIR)/src/ast/pattern
