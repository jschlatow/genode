#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# tactic files
SRC_CC = tactical.cpp \
	tactic.cpp \
	replace_proof_converter.cpp \
	proof_converter.cpp \
	probe.cpp \
	model_converter.cpp \
	horn_subsume_model_converter.cpp \
	goal_util.cpp \
	goal_shared_occs.cpp \
	goal_num_occurs.cpp \
	goal.cpp \
	filter_model_converter.cpp \
	extension_model_converter.cpp \
	equiv_proof_converter.cpp

LIBS = stdcxx z3-ast z3-model

vpath %.cpp $(Z3_DIR)/src/tactic
