#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# substitution files
SRC_CC = unifier.cpp \
	substitution_tree.cpp \
	substitution.cpp \
	matcher.cpp

LIBS = stdcxx z3-ast z3-rewriter gmp

vpath %.cpp $(Z3_DIR)/src/ast/substitution
