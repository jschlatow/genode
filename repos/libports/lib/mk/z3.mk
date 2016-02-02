#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# util files
SRC_CC   = dll.cpp \
			  gparams_register_modules.cpp \
			  install_tactic.cpp \
			  mem_initializer.cpp

LIBS     = stdcxx z3-api z3-sat z3-extra_cmds gmp

vpath %.cpp $(Z3_DIR)/src/api/dll
vpath %.cpp $(REP_DIR)/src/lib/z3

SHARED_LIB = yes
