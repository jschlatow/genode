#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# subpaving files
SRC_CC = subpaving_mpq.cpp \
	subpaving_mpfx.cpp \
	subpaving_mpff.cpp \
	subpaving_mpf.cpp \
	subpaving_hwf.cpp \
	subpaving.cpp

LIBS = stdcxx z3-interval

vpath %.cpp $(Z3_DIR)/src/math/subpaving
