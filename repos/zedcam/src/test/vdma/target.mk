# \brief  Test for VDMA on zynq board
# \author Mark Albers
# \date   2015-04-13

TARGET = vdma-test
SRC_CC += main.cc
LIBS += base

vpath main.cc $(PRG_DIR)
