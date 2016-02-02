#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# realclosure files
SRC_CC = realclosure.cpp \
	mpz_matrix.cpp

LIBS = stdcxx z3-interval

vpath %.cpp $(Z3_DIR)/src/math/realclosure
