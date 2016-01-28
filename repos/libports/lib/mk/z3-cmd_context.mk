#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# cmd_context files
SRC_CC = tactic_manager.cpp \
	tactic_cmds.cpp \
	simplify_cmd.cpp \
	pdecl.cpp \
	parametric_cmd.cpp \
	interpolant_cmds.cpp \
	eval_cmd.cpp \
	echo_tactic.cpp \
	context_params.cpp \
	cmd_util.cpp \
	cmd_context_to_goal.cpp \
	cmd_context.cpp \
	check_logic.cpp \
	basic_cmds.cpp

LIBS = stdcxx z3-solver z3-rewriter z3-interp gmp

vpath %.cpp $(Z3_DIR)/src/cmd_context
