#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# opt files
SRC_CC = wmax.cpp \
	pb_sls.cpp \
	optsmt.cpp \
	opt_solver.cpp \
	opt_pareto.cpp \
	opt_context.cpp \
	opt_cmds.cpp \
	mus.cpp \
	mss.cpp \
	maxsmt.cpp \
	maxsls.cpp \
	maxres.cpp \
	maxhs.cpp \
	hitting_sets.cpp \
	fu_malik.cpp \
	bcd2.cpp

LIBS = stdcxx z3-smt z3-smtlogic_tactics z3-sls_tactic z3-sat_solver

vpath %.cpp $(Z3_DIR)/src/opt
