TARGET   = sd_card_drv
REQUIRES = zynq
SRC_CC   = main.cc
LIBS     = base server
INC_DIR += $(PRG_DIR) $(REP_DIR)/src/drivers/sd_card $(PRG_DIR)/../rpi
