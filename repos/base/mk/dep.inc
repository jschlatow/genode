#
# Common parts of dep_prg.mk, dep_lib.mk, and dep_abi.mk
#
# The target/library description file is already included.
# Hence, the LIBS variable contains the library dependencies.
#

.NOTPARALLEL:

##
# Return path to symbol file for a given library name
#
_symbol_file = $(firstword $(wildcard $(addsuffix /$1,\
                  $(foreach REP,$(REPOSITORIES),$(REP)/lib/symbols))))

include $(LIB_PROGRESS_LOG)

#
# Categorize LIBS into ABIs and actual libraries. Dependencies of libraries
# must be recursively discovered whereas ABIs don't have any dependencies.
#
LIBS_TO_VISIT := $(filter-out $(LIBS_READY),$(LIBS))
ABIS_TO_VISIT := $(foreach L,$(LIBS_TO_VISIT),$(if $(call _symbol_file,$L),$L))
LIBS_TO_VISIT := $(filter-out $(ABIS_TO_VISIT),$(LIBS_TO_VISIT))

#
# Pattern rules for generating ABI and library dependencies for var/libdeps
#

generate_abi_dep.%: log_progress
	$(VERBOSE_MK)$(MAKE) $(VERBOSE_DIR) -f $(BASE_DIR)/mk/dep_abi.mk SYMBOLS=$(call _symbol_file,$*)

generate_lib_dep.%: log_progress
	$(VERBOSE_MK)$(MAKE) $(VERBOSE_DIR) -f $(BASE_DIR)/mk/dep_lib.mk REP_DIR=$(REP_DIR) LIB=$*

generate_missing_ports:
	$(if $(DEP_MISSING_PORTS),\
	   @echo "MISSING_PORTS += $(DEP_MISSING_PORTS)" >> $(LIB_DEP_FILE),\
	   @true)

generate_dep_lists:
	@(echo "$(DEP_A_VAR_NAME)  = $(foreach l,$(LIBS),\$${ARCHIVE_NAME($l)} \$$(DEP_A_$l))"; \
	  echo "$(DEP_SO_VAR_NAME) = $(foreach l,$(LIBS),\$${SO_NAME($l)} \$$(DEP_SO_$l))"; \
	  echo "") >> $(LIB_DEP_FILE)

log_progress:

generate: log_progress
generate: $(addprefix generate_abi_dep.,$(ABIS_TO_VISIT))
generate: $(addprefix generate_lib_dep.,$(LIBS_TO_VISIT))
generate: generate_missing_ports $(if $(LIBS),generate_dep_lists)

UNSATISFIED_REQUIREMENTS = $(filter-out $(SPECS),$(REQUIRES))
