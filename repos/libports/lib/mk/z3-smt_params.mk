#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# smt_params files
SRC_CC = theory_pb_params.cpp \
	theory_bv_params.cpp \
	theory_array_params.cpp \
	theory_arith_params.cpp \
	smt_params.cpp \
	qi_params.cpp \
	preprocessor_params.cpp \
	dyn_ack_params.cpp

LIBS = stdcxx z3-ast z3-simplifier z3-pattern z3-bit_blaster

vpath %.cpp $(Z3_DIR)/src/smt/params
