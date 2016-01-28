#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# sls_tactic files
SRC_CC = sls_tactic.cpp \
	sls_engine.cpp \
	bvsls_opt_engine.cpp

LIBS = stdcxx z3-tactic z3-normal_forms z3-core_tactics z3-bv_tactics gmp

vpath %.cpp $(Z3_DIR)/src/tactic/sls
