#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# simplifier files
SRC_CC = simplifier_plugin.cpp \
	simplifier.cpp \
	push_app_ite.cpp \
	pull_ite_tree.cpp \
	poly_simplifier_plugin.cpp \
	maximise_ac_sharing.cpp \
	inj_axiom.cpp \
	fpa_simplifier_plugin.cpp \
	elim_bounds.cpp \
	distribute_forall.cpp \
	datatype_simplifier_plugin.cpp \
	bv_simplifier_plugin.cpp \
	bv_simplifier_params.cpp \
	bv_elim.cpp \
	bit2int.cpp \
	basic_simplifier_plugin.cpp \
	array_simplifier_plugin.cpp \
	array_simplifier_params.cpp \
	arith_simplifier_plugin.cpp \
	arith_simplifier_params.cpp

LIBS = stdcxx z3-rewriter

vpath %.cpp $(Z3_DIR)/src/ast/simplifier
