LIB_DIR     = $(REP_DIR)/src/lib/audio
LIB_INC_DIR = $(LIB_DIR)/include

AUDIO_CONTRIB_DIR := $(call select_from_ports,dde_bsd)/src/lib/audio

#
# Set include paths up before adding the dde_bsd_audio_include library
# because it will use INC_DIR += and must be at the end
#
INC_DIR += $(LIB_DIR)
INC_DIR += $(LIB_INC_DIR)
INC_DIR += $(AUDIO_CONTRIB_DIR)

LIBS += dde_bsd_audio_include

SRC_C  := bsd_emul_pci.c
SRC_CC += pci.cc

CC_OPT += -Wno-unused-but-set-variable

# disable builtins
CC_OPT += -fno-builtin-printf -fno-builtin-snprintf -fno-builtin-vsnprintf \
          -fno-builtin-malloc -fno-builtin-free -fno-builtin-log -fno-builtin-log2

CC_OPT += -D_KERNEL

# enable when debugging
#CC_OPT += -DAC97_DEBUG
#CC_OPT += -DAUICH_DEBUG
#CC_OPT += -DAZALIA_DEBUG
#CC_OPT += -DDIAGNOSTIC

# AC97 codec
SRC_C += dev/ic/ac97.c

# HDA driver
SRC_C += dev/pci/azalia.c dev/pci/azalia_codec.c

# ICH driver
SRC_C += dev/pci/auich.c

# ES1370
SRC_C += dev/pci/eap.c

vpath %.c  $(AUDIO_CONTRIB_DIR)
vpath %.c  $(LIB_DIR)
vpath %.cc $(LIB_DIR)

# vi: set ft=make :
