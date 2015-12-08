TARGET   = test-lemon_lgf
LIBS     = lemon stdcxx 
LEMON    = $(call select_from_ports,lemon)/src/lib/lemon/demo
SRC_CC   = lgf_demo.cc

vpath lgf_demo.cc $(LEMON)
