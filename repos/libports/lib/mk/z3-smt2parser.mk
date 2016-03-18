#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# smt2parser files
SRC_CC = smt2scanner.cpp \
	smt2parser.cpp

LIBS = stdcxx z3-cmd_context z3-parser_util

vpath %.cpp $(Z3_DIR)/src/parsers/smt2
