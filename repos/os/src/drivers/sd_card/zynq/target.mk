TARGET   = sd_card_drv
REQUIRES = hw_zynq
SRC_CC   = main.cc
LIBS     = base server
INC_DIR += $(PRG_DIR) $(PRG_DIR)/.. $(PRG_DIR)/../rpi
