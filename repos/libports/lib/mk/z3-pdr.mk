#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# pdr files
SRC_CC = pdr_util.cpp \
	pdr_sym_mux.cpp \
	pdr_smt_context_manager.cpp \
	pdr_reachable_cache.cpp \
	pdr_prop_solver.cpp \
	pdr_manager.cpp \
	pdr_generalizers.cpp \
	pdr_farkas_learner.cpp \
	pdr_dl_interface.cpp \
	pdr_context.cpp \
	pdr_closure.cpp

LIBS = stdcxx z3-muz z3-transforms z3-arith_tactics z3-core_tactics z3-smt_tactic

vpath %.cpp $(Z3_DIR)/src/muz/pdr
