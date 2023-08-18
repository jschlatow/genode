TARGET   = pc_platform_drv
REQUIRES = x86

include $(call select_from_repositories,src/drivers/platform/target.inc)

SRC_CC += intel/root_table.cc
SRC_CC += intel/context_table.cc
SRC_CC += intel/managed_root_table.cc
SRC_CC += intel/io_mmu.cc
