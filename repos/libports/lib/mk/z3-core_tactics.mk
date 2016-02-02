#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# core_tactics files
SRC_CC = tseitin_cnf_tactic.cpp \
	symmetry_reduce_tactic.cpp \
	split_clause_tactic.cpp \
	solve_eqs_tactic.cpp \
	simplify_tactic.cpp \
	reduce_args_tactic.cpp \
	propagate_values_tactic.cpp \
	pb_preprocess_tactic.cpp \
	occf_tactic.cpp \
	nnf_tactic.cpp \
	elim_uncnstr_tactic.cpp \
	elim_term_ite_tactic.cpp \
	distribute_forall_tactic.cpp \
	der_tactic.cpp \
	ctx_simplify_tactic.cpp \
	cofactor_term_ite_tactic.cpp \
	cofactor_elim_term_ite.cpp \
	blast_term_ite_tactic.cpp

LIBS = stdcxx z3-tactic z3-normal_forms

vpath %.cpp $(Z3_DIR)/src/tactic/core
