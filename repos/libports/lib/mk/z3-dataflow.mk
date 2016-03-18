#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# dataflow files
SRC_CC = dataflow.cpp

LIBS = stdcxx z3-muz

vpath %.cpp $(Z3_DIR)/src/muz/dataflow
