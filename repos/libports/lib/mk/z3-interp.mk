#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# interp files
SRC_CC = iz3translate_direct.cpp \
	iz3translate.cpp \
	iz3scopes.cpp \
	iz3proof_itp.cpp \
	iz3proof.cpp \
	iz3profiling.cpp \
	iz3pp.cpp \
	iz3mgr.cpp \
	iz3interp.cpp \
	iz3foci.cpp \
	iz3checker.cpp \
	iz3base.cpp

LIBS = stdcxx z3-solver

vpath %.cpp $(Z3_DIR)/src/interp
