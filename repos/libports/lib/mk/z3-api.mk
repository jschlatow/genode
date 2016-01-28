#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# api files
SRC_CC = api_commands.cpp \
	api_log_macros.cpp \
	z3_replayer.cpp \
	api_tactic.cpp \
	api_stats.cpp \
	api_solver.cpp \
	api_seq.cpp \
	api_rcf.cpp \
	api_quant.cpp \
	api_polynomial.cpp \
	api_pb.cpp \
	api_parsers.cpp \
	api_params.cpp \
	api_opt.cpp \
	api_numeral.cpp \
	api_model.cpp \
	api_log.cpp \
	api_interp.cpp \
	api_goal.cpp \
	api_fpa.cpp \
	api_datatype.cpp \
	api_datalog.cpp \
	api_context.cpp \
	api_config_params.cpp \
	api_bv.cpp \
	api_ast_vector.cpp \
	api_ast_map.cpp \
	api_ast.cpp \
	api_array.cpp \
	api_arith.cpp \
	api_algebraic.cpp

LIBS = stdcxx z3-portfolio z3-smtparser z3-realclosure z3-interp z3-opt gmp

vpath %.cpp $(Z3_DIR)/src/api
vpath api_commands.cpp $(REP_DIR)/src/lib/z3
vpath api_log_macros.cpp $(REP_DIR)/src/lib/z3
