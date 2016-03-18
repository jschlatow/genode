#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# normal_forms files
SRC_CC = pull_quant.cpp \
	nnf.cpp \
	name_exprs.cpp \
	defined_names.cpp

LIBS = stdcxx z3-rewriter

vpath %.cpp $(Z3_DIR)/src/ast/normal_forms
