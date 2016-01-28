#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# ast files
SRC_CC = well_sorted.cpp \
	used_vars.cpp \
	static_features.cpp \
	shared_occs.cpp \
	seq_decl_plugin.cpp \
	reg_decl_plugins.cpp \
	pp.cpp \
	pb_decl_plugin.cpp \
	occurs.cpp \
	num_occurs.cpp \
	macro_substitution.cpp \
	has_free_vars.cpp \
	func_decl_dependencies.cpp \
	fpa_decl_plugin.cpp \
	format.cpp \
	for_each_expr.cpp \
	for_each_ast.cpp \
	expr_substitution.cpp \
	expr_stat.cpp \
	expr_map.cpp \
	expr_functors.cpp \
	expr_abstract.cpp \
	expr2var.cpp \
	expr2polynomial.cpp \
	dl_decl_plugin.cpp \
	decl_collector.cpp \
	datatype_decl_plugin.cpp \
	bv_decl_plugin.cpp \
	ast_util.cpp \
	ast_translation.cpp \
	ast_smt_pp.cpp \
	ast_smt2_pp.cpp \
	ast_printer.cpp \
	ast_pp_util.cpp \
	ast_lt.cpp \
	ast_ll_pp.cpp \
	ast.cpp \
	array_decl_plugin.cpp \
	arith_decl_plugin.cpp \
	act_cache.cpp

LIBS = stdcxx z3-util z3-polynomial gmp

vpath %.cpp $(Z3_DIR)/src/ast
