#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# proof_checker files
SRC_CC = proof_checker.cpp

LIBS = stdcxx z3-rewriter

vpath %.cpp $(Z3_DIR)/src/ast/proof_checker
