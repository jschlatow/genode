#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# muz files
SRC_CC = rule_properties.cpp \
	proof_utils.cpp \
	hnf.cpp \
	dl_util.cpp \
	dl_rule_transformer.cpp \
	dl_rule_subsumption_index.cpp \
	dl_rule_set.cpp \
	dl_rule.cpp \
	dl_costs.cpp \
	dl_context.cpp \
	dl_boogie_proof.cpp \
	bind_variables.cpp

LIBS = stdcxx z3-smt z3-sat z3-smt2parser z3-aig_tactic z3-qe gmp

vpath %.cpp $(Z3_DIR)/src/muz/base
