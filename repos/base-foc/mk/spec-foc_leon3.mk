SPECS += foc_sparc platform_leon3

include $(call select_from_repositories,mk/spec-platform_leon3.mk)
include $(call select_from_repositories,mk/spec-foc_sparc.mk)
