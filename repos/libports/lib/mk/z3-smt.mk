#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# smt files
SRC_CC = watch_list.cpp \
	uses_theory.cpp \
	theory_wmaxsat.cpp \
	theory_utvpi.cpp \
	theory_seq.cpp \
	theory_pb.cpp \
	theory_opt.cpp \
	theory_fpa.cpp \
	theory_dummy.cpp \
	theory_dl.cpp \
	theory_diff_logic.cpp \
	theory_dense_diff_logic.cpp \
	theory_datatype.cpp \
	theory_bv.cpp \
	theory_array_full.cpp \
	theory_array_base.cpp \
	theory_array.cpp \
	theory_arith.cpp \
	smt_value_sort.cpp \
	smt_theory.cpp \
	smt_statistics.cpp \
	smt_solver.cpp \
	smt_setup.cpp \
	smt_relevancy.cpp \
	smt_quick_checker.cpp \
	smt_quantifier_stat.cpp \
	smt_quantifier.cpp \
	smt_model_generator.cpp \
	smt_model_finder.cpp \
	smt_model_checker.cpp \
	smt_literal.cpp \
	smt_kernel.cpp \
	smt_justification.cpp \
	smt_internalizer.cpp \
	smt_implied_equalities.cpp \
	smt_for_each_relevant_expr.cpp \
	smt_farkas_util.cpp \
	smt_enode.cpp \
	smt_context_stat.cpp \
	smt_context_pp.cpp \
	smt_context_inv.cpp \
	smt_context.cpp \
	smt_conflict_resolution.cpp \
	smt_clause.cpp \
	smt_checker.cpp \
	smt_cg_table.cpp \
	smt_case_split_queue.cpp \
	smt_almost_cg_table.cpp \
	qi_queue.cpp \
	old_interval.cpp \
	mam.cpp \
	fingerprints.cpp \
	expr_context_simplifier.cpp \
	elim_term_ite.cpp \
	dyn_ack.cpp \
	cost_evaluator.cpp \
	cached_var_subst.cpp \
	asserted_formulas.cpp \
	arith_eq_solver.cpp \
	arith_eq_adapter.cpp

LIBS = stdcxx z3-bit_blaster z3-macros z3-normal_forms z3-cmd_context z3-proto_model z3-substitution z3-grobner z3-euclid z3-simplex z3-proof_checker z3-pattern z3-parser_util z3-fpa

vpath %.cpp $(Z3_DIR)/src/smt
