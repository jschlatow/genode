#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# parser_util files
SRC_CC = simple_parser.cpp \
	scanner.cpp \
	pattern_validation.cpp \
	cost_parser.cpp

LIBS = stdcxx z3-ast gmp

vpath %.cpp $(Z3_DIR)/src/parsers/util
