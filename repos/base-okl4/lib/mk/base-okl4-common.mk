#
# \brief  Portions of base library shared by core and non-core processes
# \author Norman Feske
# \date   2013-02-14
#

include $(BASE_DIR)/lib/mk/base-common.inc

LIBS += startup-okl4 syscall-okl4

SRC_CC += capability.cc capability_raw.cc
SRC_CC += rpc_dispatch_loop.cc
SRC_CC += thread.cc thread_bootstrap.cc thread_myself.cc
SRC_CC += stack_area_addr.cc
