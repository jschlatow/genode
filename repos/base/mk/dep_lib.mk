#
# This file determines dependencies of a library from other libraries
#
# The following variables must be defined by the caller:
#
#   VERBOSE_DIR      - verboseness of directory change messages
#   REPOSITORIES     - source code repositories to use
#   BASE_DIR         - base directory of build system repository
#   BUILD_BASE_DIR   - build directory with build config
#   LIB_PROGRESS_LOG - library build log file
#

ACCUMULATE_MISSING_PORTS = 1

#
# Include common utility functions
#
include $(BASE_DIR)/mk/util.inc

#
# Generate dependencies only for those libs that are
# not already contained in the library build log
#
include $(LIB_PROGRESS_LOG)
ifneq ($(filter $(LIB),$(LIBS_READY)),)
already_visited:
	@true
else
all: append_lib_to_progress_log
endif

append_lib_to_progress_log:
	@echo "LIBS_READY  += $(LIB)" >> $(LIB_PROGRESS_LOG)

LIB_MK_DIRS  = $(foreach REP,$(REPOSITORIES),$(addprefix $(REP)/lib/mk/spec/,     $(SPECS)) $(REP)/lib/mk)
SYMBOLS_DIRS = $(foreach REP,$(REPOSITORIES),$(addprefix $(REP)/lib/symbols/spec/,$(SPECS)) $(REP)/lib/symbols)

#
# Of all possible file locations, use the (first) one that actually exist.
#
LIB_MK  = $(firstword $(wildcard $(addsuffix /$(LIB).mk,$(LIB_MK_DIRS))))
SYMBOLS = $(firstword $(wildcard $(addsuffix /$(LIB),   $(SYMBOLS_DIRS))))

ifneq ($(SYMBOLS),)
SHARED_LIB := yes
endif

#
# Sanity check to detect missing library-description file
#
ifeq ($(sort $(LIB_MK) $(SYMBOLS)),)
all: warn_missing_lib_mk
else
all: check_unsatisfied_requirements
endif

warn_missing_lib_mk: generate_lib_rule_for_defect_library
	@$(ECHO) "Library-description file $(DARK_COL)$(LIB).mk$(DEFAULT_COL) is missing"

#
# Determine the repository base directory from the absolute pathname
# of the choosen libname.mk file. We need to specify the override
# command because REP_DIR was first set by prg.mk when building a
# program target. The repository of a library could be a different
# one.
#
# Finally, we strip the trailing slash from the path. The slash was
# used to match the whole directory in 'findstring'.
#
override REP_DIR := $(firstword $(foreach REP,$(REPOSITORIES),$(findstring $(REP)/,$(LIB_MK))))
override REP_DIR := $(REP_DIR:/=)

include $(BASE_DIR)/mk/base-libs.mk
include $(LIB_MK)

ifdef SHARED_LIB
BUILD_ARTIFACTS ?= $(LIB).lib.so
endif

ifdef SHARED_LIB
LIBS += ldso_so_support
endif

#
# Hide archive dependencies of shared libraries from users of the shared
# library. Library users examine the 'DEP_A_<lib>' variable to determine
# transitive dependencies. For shared libraries, this variable remains
# undefined.
#
ifdef SHARED_LIB
DEP_A_VAR_NAME  := PRIVATE_DEP_A_$(LIB)
DEP_SO_VAR_NAME := PRIVATE_DEP_SO_$(LIB)
else
DEP_A_VAR_NAME  := DEP_A_$(LIB)
DEP_SO_VAR_NAME := DEP_SO_$(LIB)
endif

#
# Check if the requirements of the target are satisfied
#
UNSATISFIED_REQUIREMENTS = $(filter-out $(SPECS),$(REQUIRES))
ifneq ($(UNSATISFIED_REQUIREMENTS),)
check_unsatisfied_requirements: warn_unsatisfied_requirements
else
check_unsatisfied_requirements: generate_lib_rule
endif

warn_unsatisfied_requirements: generate_lib_rule_for_defect_library
	@$(ECHO) "Skip library $(LIB) because it requires $(DARK_COL)$(UNSATISFIED_REQUIREMENTS)$(DEFAULT_COL)"

generate_lib_rule_for_defect_library:
	@(echo "INVALID_DEPS += $(LIB)"; \
	  echo "$(LIB).lib:"; \
	  echo "") >> $(LIB_DEP_FILE)

LIBS_TO_VISIT = $(filter-out $(LIBS_READY),$(LIBS))

generate_lib_rule:
ifneq ($(DEP_MISSING_PORTS),)
	@(echo "MISSING_PORTS += $(DEP_MISSING_PORTS)"; \
	  echo "") >> $(LIB_DEP_FILE)
endif
	@for i in $(LIBS_TO_VISIT); do \
	  $(MAKE) $(VERBOSE_DIR) -f $(BASE_DIR)/mk/dep_lib.mk REP_DIR=$(REP_DIR) LIB=$$i; done
ifneq ($(LIBS),)
	@(echo "$(DEP_A_VAR_NAME)  = $(foreach l,$(LIBS),\$${ARCHIVE_NAME($l)} \$$(DEP_A_$l))"; \
	  echo "$(DEP_SO_VAR_NAME) = $(foreach l,$(LIBS),\$${SO_NAME($l)} \$$(DEP_SO_$l))"; \
	  echo "") >> $(LIB_DEP_FILE)
endif
ifdef SHARED_LIB
	@(echo "$(LIB).lib.a:"; \
	  echo "	@true"; \
	  echo "$(LIB).lib.so: check_ports $(LIB).abi.so $(addsuffix .lib.a,$(LIBS)) $(addsuffix .abi.so,$(LIBS))"; \
	  echo "	\$$(VERBOSE)\$$(call _prepare_lib_step,\$$@,$(LIB),$(BUILD_ARTIFACTS))"; \
	  echo "	\$$(VERBOSE_MK)\$$(MAKE) \$$(VERBOSE_DIR) -C \$$(LIB_CACHE_DIR)/$(LIB) -f \$$(BASE_DIR)/mk/so.mk \\"; \
	  echo "	     REP_DIR=$(REP_DIR) \\"; \
	  echo "	     LIB_MK=$(LIB_MK) \\"; \
	  echo "	     SYMBOLS=$(SYMBOLS) \\"; \
	  echo "	     LIB=$(LIB) \\"; \
	  echo "	     ARCHIVES=\"\$$(sort \$$($(DEP_A_VAR_NAME)))\" \\"; \
	  echo "	     SHARED_LIBS=\"\$$(sort \$$($(DEP_SO_VAR_NAME)))\" \\"; \
	  echo "	     BUILD_BASE_DIR=$(BUILD_BASE_DIR) \\"; \
	  echo "	     SHELL=$(SHELL) \\"; \
	  echo "	     INSTALL_DIR=\$$(INSTALL_DIR) \\"; \
	  echo "	     DEBUG_DIR=\$$(DEBUG_DIR)"; \
	  echo "$(LIB).abi.so:"; \
	  echo "	\$$(VERBOSE)\$$(call _prepare_lib_step,\$$@,$(LIB),)"; \
	  echo "	\$$(VERBOSE_MK)\$$(MAKE) \$$(VERBOSE_DIR) -C \$$(LIB_CACHE_DIR)/$(LIB) -f \$$(BASE_DIR)/mk/abi.mk \\"; \
	  echo "	     SYMBOLS=$(SYMBOLS) \\"; \
	  echo "	     LIB=$(LIB) \\"; \
	  echo "	     SHELL=$(SHELL)"; \
	  echo "SO_NAME($(LIB)) := $(LIB).lib.so"; \
	  echo "") >> $(LIB_DEP_FILE)
else # not SHARED_LIB
	@(echo "$(LIB).lib.a: check_ports $(addsuffix .lib.a,$(LIBS)) $(addsuffix .abi.so,$(LIBS))"; \
	  echo "	\$$(VERBOSE)\$$(call _prepare_lib_step,\$$@,$(LIB),$(BUILD_ARTIFACTS))"; \
	  echo "	\$$(VERBOSE_MK)\$$(MAKE) \$$(VERBOSE_DIR) -C \$$(LIB_CACHE_DIR)/$(LIB) -f \$$(BASE_DIR)/mk/a.mk \\"; \
	  echo "	     REP_DIR=$(REP_DIR) \\"; \
	  echo "	     LIB_MK=$(LIB_MK) \\"; \
	  echo "	     LIB=$(LIB) \\"; \
	  echo "	     ARCHIVES=\"\$$(sort \$$($(DEP_A_VAR_NAME)))\" \\"; \
	  echo "	     SHARED_LIBS=\"\$$(sort \$$($(DEP_SO_VAR_NAME)))\" \\"; \
	  echo "	     BUILD_BASE_DIR=$(BUILD_BASE_DIR) \\"; \
	  echo "	     SHELL=$(SHELL) \\"; \
	  echo "	     INSTALL_DIR=\$$(INSTALL_DIR) \\"; \
	  echo "	     DEBUG_DIR=\$$(DEBUG_DIR)"; \
	  echo "$(LIB).lib.so $(LIB).abi.so:"; \
	  echo "	@true"; \
	  echo "ARCHIVE_NAME($(LIB)) := $(LIB).lib.a"; \
	  echo "") >> $(LIB_DEP_FILE)
endif

