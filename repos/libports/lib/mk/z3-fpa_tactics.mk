#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# fpa_tactics files
SRC_CC = qffp_tactic.cpp \
	fpa2bv_tactic.cpp \
	fpa2bv_model_converter.cpp

LIBS = stdcxx z3-fpa z3-core_tactics z3-bv_tactics z3-sat_tactic z3-smt_tactic z3-arith_tactics z3-smtlogic_tactics gmp

vpath %.cpp $(Z3_DIR)/src/tactic/fpa
