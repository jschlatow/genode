#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# arith_tactics files
SRC_CC = recover_01_tactic.cpp \
	purify_arith_tactic.cpp \
	propagate_ineqs_tactic.cpp \
	probe_arith.cpp \
	pb2bv_tactic.cpp \
	pb2bv_model_converter.cpp \
	normalize_bounds_tactic.cpp \
	nla2bv_tactic.cpp \
	linear_equation.cpp \
	lia2pb_tactic.cpp \
	lia2card_tactic.cpp \
	fm_tactic.cpp \
	fix_dl_var_tactic.cpp \
	factor_tactic.cpp \
	eq2bv_tactic.cpp \
	elim01_tactic.cpp \
	diff_neq_tactic.cpp \
	degree_shift_tactic.cpp \
	card2bv_tactic.cpp \
	bv2real_rewriter.cpp \
	bv2int_rewriter.cpp \
	bound_propagator.cpp \
	bound_manager.cpp \
	arith_bounds_tactic.cpp \
	add_bounds_tactic.cpp

LIBS = stdcxx z3-core_tactics z3-sat gmp

vpath %.cpp $(Z3_DIR)/src/tactic/arith
