# \brief  Test for I2C on zynq board
# \author Mark Albers
# \date   2015-03-30

TARGET = i2c-test
SRC_CC += main.cc
LIBS += base

vpath main.cc $(PRG_DIR)
