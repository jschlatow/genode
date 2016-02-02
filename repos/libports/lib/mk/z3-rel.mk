#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# rel files
SRC_CC = udoc_relation.cpp \
	tbv.cpp \
	rel_context.cpp \
	karr_relation.cpp \
	doc.cpp \
	dl_table_relation.cpp \
	dl_table.cpp \
	dl_sparse_table.cpp \
	dl_sieve_relation.cpp \
	dl_relation_manager.cpp \
	dl_product_relation.cpp \
	dl_mk_simple_joins.cpp \
	dl_mk_similarity_compressor.cpp \
	dl_mk_partial_equiv.cpp \
	dl_mk_explanations.cpp \
	dl_lazy_table.cpp \
	dl_interval_relation.cpp \
	dl_instruction.cpp \
	dl_finite_product_relation.cpp \
	dl_external_relation.cpp \
	dl_compiler.cpp \
	dl_check_table.cpp \
	dl_bound_relation.cpp \
	dl_base.cpp \
	check_relation.cpp \
	aig_exporter.cpp

LIBS = stdcxx z3-muz z3-transforms

vpath %.cpp $(Z3_DIR)/src/muz/rel
