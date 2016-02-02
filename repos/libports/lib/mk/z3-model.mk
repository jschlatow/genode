#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# model files
SRC_CC = model_v2_pp.cpp \
	model_smt2_pp.cpp \
	model_pp.cpp \
	model_implicant.cpp \
	model_evaluator.cpp \
	model_core.cpp \
	model2expr.cpp \
	model.cpp \
	func_interp.cpp

LIBS = stdcxx z3-rewriter

vpath %.cpp $(Z3_DIR)/src/model
