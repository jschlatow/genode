TARGET   = pc_platform_drv
REQUIRES = x86_64

include $(call select_from_repositories,src/drivers/platform/target.inc)

SRC_CC += intel/root_table.cc
SRC_CC += intel/context_table.cc
SRC_CC += intel/managed_root_table.cc
SRC_CC += intel/io_mmu.cc
SRC_CC += intel/page_table.cc

INC_DIR += $(PRG_DIR)/../../

vpath intel/%.cc $(PRG_DIR)/../../
