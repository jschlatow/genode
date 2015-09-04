#
# Enable peripherals of the platform
#
SPECS += cadence_gem

#
# Pull in CPU specifics
#
SPECS += zynq

REP_INC_DIR += include/platform/parallella

include $(call select_from_repositories,mk/spec-zynq.mk)
