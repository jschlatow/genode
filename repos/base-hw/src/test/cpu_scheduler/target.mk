#
# \brief  Build config for a core that tests its CPU-scheduler implementation
# \author Martin Stein
# \date   2011-12-16
#

TARGET   = test-cpu_scheduler
SRC_CC   = test.cc quota_scheduler.cc double_list.cc
INC_DIR  = $(REP_DIR)/src/core $(REP_DIR)/src/lib $(BASE_DIR)/src/include
LIBS     = base

INC_DIR += $(BASE_DIR)/../base-hw/src/core/spec/quota_scheduler/

vpath test.cc $(PRG_DIR)
vpath quota_scheduler.cc    $(REP_DIR)/src/core/spec/quota_scheduler/kernel
vpath %.cc    $(REP_DIR)/src/core/kernel
