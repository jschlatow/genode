#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# smtparser files
SRC_CC = smtparser.cpp \
	smtlib_solver.cpp \
	smtlib.cpp

LIBS = stdcxx z3-portfolio gmp

vpath %.cpp $(Z3_DIR)/src/parsers/smt
