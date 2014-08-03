REQUIRES       = platform_leon3
FIASCO_DIR    := $(call select_from_ports,foc)/src/kernel/foc/kernel/fiasco
KERNEL_CONFIG  = $(REP_DIR)/config/leon3.kernel

-include $(PRG_DIR)/../target.inc
