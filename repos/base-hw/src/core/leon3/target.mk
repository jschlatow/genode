#
# \brief  Build config for Genodes core process
# \author Johannes Schlatow
# \author Norman Feske
# \date   2013-04-05
#

# declare which specs must be given to build this target
REQUIRES = hw_leon3

# add include paths
INC_DIR += $(REP_DIR)/src/core/leon3

## add C++ sources
SRC_CC += platform_services.cc \
          cpu_support.cc
#          platform_support.cc \

# add assembly sources
#SRC_S += mode_transition.s \
#         boot_modules.s \
#         crt0.s
SRC_S += boot_modules.s \
			crt0.s

# declare source paths
vpath platform_services.cc $(BASE_DIR)/src/core
#vpath platform_support.cc  $(REP_DIR)/src/core/leon3
#vpath mode_transition.s    $(REP_DIR)/src/core/leon3
#vpath cpu_support.cc       $(REP_DIR)/src/core/leon3
vpath crt0.s               $(REP_DIR)/src/core/leon3

#
# Check if there are other images which shall be linked to core.
# If not use a dummy boot-modules file which includes only the symbols.
#
ifeq ($(wildcard $(BUILD_BASE_DIR)/boot_modules.s),)
  vpath boot_modules.s $(REP_DIR)/src/core/leon3
else
  INC_DIR += $(BUILD_BASE_DIR)
  vpath boot_modules.s $(BUILD_BASE_DIR)
endif

# include less specific target parts
include $(REP_DIR)/src/core/target.inc

