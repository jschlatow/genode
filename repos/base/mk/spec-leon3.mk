#
# LEON3-specific Genode headers
#
REP_INC_DIR += include/leon3

SPECS += sparc

include $(call select_from_repositories,mk/spec-sparc.mk)
