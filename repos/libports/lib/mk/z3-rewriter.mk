#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# rewriter files
SRC_CC = var_subst.cpp \
	th_rewriter.cpp \
	seq_rewriter.cpp \
	rewriter.cpp \
	quant_hoist.cpp \
	pb_rewriter.cpp \
	mk_simplified_app.cpp \
	fpa_rewriter.cpp \
	factor_rewriter.cpp \
	expr_safe_replace.cpp \
	expr_replacer.cpp \
	dl_rewriter.cpp \
	der.cpp \
	datatype_rewriter.cpp \
	bv_rewriter.cpp \
	bool_rewriter.cpp \
	ast_counter.cpp \
	array_rewriter.cpp \
	arith_rewriter.cpp

LIBS = stdcxx z3-ast z3-polynomial z3-automata gmp

vpath %.cpp $(Z3_DIR)/src/ast/rewriter
