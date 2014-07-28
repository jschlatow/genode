#
# \brief  Build-system configurations for LEON3
# \author Johannes Schlatow
# \date   2014-07-23
#

# denote wich specs are also fullfilled by this spec
SPECS += leon3

# add repository relative include paths
REP_INC_DIR += include/platform/leon3

# include implied specs
include $(call select_from_repositories,mk/spec-leon3.mk)
