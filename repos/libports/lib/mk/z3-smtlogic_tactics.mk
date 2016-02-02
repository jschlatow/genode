#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# smtlogic_tactics files
SRC_CC = quant_tactics.cpp \
	qfufnra_tactic.cpp \
	qfufbv_tactic.cpp \
	qfuf_tactic.cpp \
	qfnra_tactic.cpp \
	qfnia_tactic.cpp \
	qflra_tactic.cpp \
	qflia_tactic.cpp \
	qfidl_tactic.cpp \
	qfbv_tactic.cpp \
	qfauflia_tactic.cpp \
	qfaufbv_tactic.cpp \
	nra_tactic.cpp

LIBS = stdcxx z3-arith_tactics z3-bv_tactics z3-nlsat_tactic z3-smt_tactic z3-aig_tactic z3-fp z3-muz z3-qe z3-nlsat_smt_tactic

vpath %.cpp $(Z3_DIR)/src/tactic/smtlogics
