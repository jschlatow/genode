# \brief  Test for Gpio on zynq board
# \author Mark Albers
# \date   2015-03-30

TARGET = gpio-test
SRC_CC += main.cc
LIBS += base

vpath main.cc $(PRG_DIR)
