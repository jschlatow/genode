# \brief  Zedcam App on zynq board
# \author Mark Albers
# \date   2015-04-21

TARGET = zedcam-app
SRC_CC += main.cc
LIBS += base

vpath main.cc $(PRG_DIR)
