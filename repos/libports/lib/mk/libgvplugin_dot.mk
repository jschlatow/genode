GRAPHVIZ_DIR = $(call select_from_ports,graphviz)/src/
LIBS    += libc libgvc libgvcommon libgvpack libcgraph
INC_DIR += $(GRAPHVIZ_DIR)/lib/gvc
INC_DIR += $(REP_DIR)/src/lib/graphviz
SRC_C   = gvlayout_dot_layout.c gvplugin_dot_layout.c

CC_OPT += -DHAVE_CONFIG_H

vpath %.c $(GRAPHVIZ_DIR)/plugin/dot_layout
vpath %.c $(GRAPHVIZ_DIR)/lib/dotgen

#SHARED_LIB = yes
