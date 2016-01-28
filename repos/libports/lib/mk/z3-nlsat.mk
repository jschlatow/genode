#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# nlsat files
SRC_CC   = nlsat_clause.cpp \
	nlsat_evaluator.cpp \
	nlsat_explain.cpp \
	nlsat_interval_set.cpp \
	nlsat_solver.cpp \
	nlsat_types.cpp

LIBS     = stdcxx z3-sat z3-polynomial gmp

vpath %.cpp  $(Z3_DIR)/src/nlsat

#SHARED_LIB = no
