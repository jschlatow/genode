THIS_DIR = $(dir $(call select_from_repositories,src/core/trace_core/init.cc))
SRC_CC += init.cc
SRC_CC += trace_core.cc

vpath init.cc ${THIS_DIR}
vpath trace_core.cc ${THIS_DIR}

# vi: set ft=make :
