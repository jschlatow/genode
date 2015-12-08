TARGET   = sd_card_drv
REQUIRES = zynq
SRC_CC   = main.cc
LIBS     = base server
INC_DIR += $(PRG_DIR) $(PRG_DIR)/.. $(PRG_DIR)/../rpi
