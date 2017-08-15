THIS_DIR = $(dir $(call select_from_repositories,src/core/trace_core/null/init.cc))

SRC_CC += init.cc

vpath init.cc ${THIS_DIR}

# vi: set ft=make :
