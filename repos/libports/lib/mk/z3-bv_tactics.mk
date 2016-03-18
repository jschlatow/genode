#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# bv_tactics files
SRC_CC = max_bv_sharing_tactic.cpp \
	elim_small_bv_tactic.cpp \
	bvarray2uf_tactic.cpp \
	bvarray2uf_rewriter.cpp \
	bv_size_reduction_tactic.cpp \
	bv1_blaster_tactic.cpp \
	bit_blaster_tactic.cpp \
	bit_blaster_model_converter.cpp

LIBS = stdcxx z3-tactic z3-bit_blaster

vpath %.cpp $(Z3_DIR)/src/tactic/bv
