#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# proto_model files
SRC_CC = value_factory.cpp \
	struct_factory.cpp \
	proto_model.cpp \
	numeral_factory.cpp \
	datatype_factory.cpp \
	array_factory.cpp

LIBS = stdcxx z3-model z3-simplifier z3-smt_params gmp

vpath %.cpp $(Z3_DIR)/src/smt/proto_model
