#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# macros files
SRC_CC = quasi_macros.cpp \
	macro_util.cpp \
	macro_manager.cpp \
	macro_finder.cpp

LIBS = stdcxx z3-simplifier gmp

vpath %.cpp $(Z3_DIR)/src/ast/macros
