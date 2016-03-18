#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# fp files
SRC_CC = horn_tactic.cpp \
	dl_register_engine.cpp \
	dl_cmds.cpp \
	datalog_parser.cpp

LIBS = stdcxx z3-muz z3-pdr z3-clp z3-tab z3-rel z3-bmc z3-duality_intf z3-ddnf

vpath %.cpp $(Z3_DIR)/src/muz/fp
