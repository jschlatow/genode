#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# ufbv_tactic files
SRC_CC = ufbv_tactic.cpp \
	ufbv_rewriter_tactic.cpp \
	ufbv_rewriter.cpp \
	quasi_macros_tactic.cpp \
	macro_finder_tactic.cpp

LIBS = stdcxx z3-normal_forms z3-core_tactics z3-macros z3-smt_tactic z3-rewriter gmp

vpath %.cpp $(Z3_DIR)/src/tactic/ufbv
