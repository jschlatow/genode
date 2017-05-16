#
# add include paths
INC_DIR += $(BASE_DIR)/../base-hw/src/core/spec/quota_priority_scheduler/
INC_DIR += $(BASE_DIR)/../base-hw/src/core/spec/quota_scheduler/

# add C++ sources
SRC_CC += spec/quota_scheduler/kernel/quota_scheduler.cc
SRC_CC += spec/quota_priority_scheduler/kernel/quota_priority_scheduler.cc
