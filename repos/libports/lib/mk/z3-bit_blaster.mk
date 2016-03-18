#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# bit_blaster files
SRC_CC = bit_blaster_rewriter.cpp \
	bit_blaster.cpp

LIBS = stdcxx z3-rewriter z3-simplifier

vpath %.cpp $(Z3_DIR)/src/ast/rewriter/bit_blaster
