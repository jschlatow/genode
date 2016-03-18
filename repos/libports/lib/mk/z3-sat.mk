#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# sat files
SRC_CC   = dimacs.cpp \
	sat_asymm_branch.cpp \
	sat_bceq.cpp \
	sat_clause.cpp \
	sat_clause_set.cpp \
	sat_clause_use_list.cpp \
	sat_cleaner.cpp \
	sat_config.cpp \
	sat_elim_eqs.cpp \
	sat_iff3_finder.cpp \
	sat_integrity_checker.cpp \
	sat_model_converter.cpp \
	sat_mus.cpp \
	sat_probing.cpp \
	sat_scc.cpp \
	sat_simplifier.cpp \
	sat_sls.cpp \
	sat_solver.cpp \
	sat_watched.cpp

LIBS     = stdcxx z3-util

vpath %.cpp  $(Z3_DIR)/src/sat

#SHARED_LIB = no
