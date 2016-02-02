#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# extra_cmds files
SRC_CC = subpaving_cmds.cpp \
	polynomial_cmds.cpp \
	dbg_cmds.cpp

LIBS = stdcxx z3-cmd_context z3-subpaving_tactic z3-arith_tactics

vpath %.cpp $(Z3_DIR)/src/cmd_context/extra_cmds
