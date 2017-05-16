#
# \brief  Build config for a core that tests its CPU-scheduler implementation
# \author Martin Stein
# \date   2011-12-16
#

TARGET   = test-quota_priority_scheduler
SRC_CC   = test.cc double_list.cc quota_scheduler.cc quota_priority_scheduler.cc
INC_DIR  = $(REP_DIR)/src/core $(REP_DIR)/src/lib $(BASE_DIR)/src/include
INC_DIR += $(REP_DIR)/src/core/spec/quota_priority_scheduler/
INC_DIR += $(REP_DIR)/src/core/spec/quota_scheduler/
LIBS     = base

vpath test.cc $(PRG_DIR)
vpath quota_scheduler.cc    $(REP_DIR)/src/core/spec/quota_scheduler/kernel
vpath quota_priority_scheduler.cc    $(REP_DIR)/src/core/spec/quota_priority_scheduler/kernel
vpath %.cc    $(REP_DIR)/src/core/kernel
