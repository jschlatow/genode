#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# ddnf files
SRC_CC = ddnf.cpp

LIBS = stdcxx z3-muz z3-transforms z3-rel gmp

vpath %.cpp $(Z3_DIR)/src/muz/ddnf
