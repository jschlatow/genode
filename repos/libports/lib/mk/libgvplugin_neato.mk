GRAPHVIZ_DIR = $(call select_from_ports,graphviz)/src/plugin
LIBS    += libc libgvc
INC_DIR += $(GRAPHVIZ_DIR)/../lib/gvc/
INC_DIR += $(REP_DIR)/src/lib/graphviz
SRC_C   = gvlayout_neato_layout.c gvplugin_neato_layout.c

CC_OPT += -DHAVE_CONFIG_H

vpath %.c $(GRAPHVIZ_DIR)/neato_layout

#SHARED_LIB = yes
