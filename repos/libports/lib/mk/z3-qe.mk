#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# qe files
SRC_CC = vsubst_tactic.cpp \
	qe_util.cpp \
	qe_tactic.cpp \
	qe_sat_tactic.cpp \
	qe_lite.cpp \
	qe_dl_plugin.cpp \
	qe_datatype_plugin.cpp \
	qe_cmd.cpp \
	qe_bv_plugin.cpp \
	qe_bool_plugin.cpp \
	qe_array_plugin.cpp \
	qe_arith_plugin.cpp \
	qe_arith.cpp \
	qe.cpp \
	nlarith_util.cpp

LIBS = stdcxx z3-smt z3-sat

vpath %.cpp $(Z3_DIR)/src/qe
