GRAPHVIZ_DIR = $(call select_from_ports,graphviz)/src/plugin
LIBS    += libc libgvc 
INC_DIR += $(GRAPHVIZ_DIR)/../lib/gvc/
INC_DIR += $(GRAPHVIZ_DIR)/../lib/cgraph/
INC_DIR += $(REP_DIR)/src/lib/graphviz
SRC_C   = gvplugin_core.c \
   gvrender_core_dot.c \
   gvrender_core_fig.c \
   gvrender_core_map.c \
   gvrender_core_ps.c \
   gvrender_core_svg.c \
   gvrender_core_tk.c \
   gvrender_core_vml.c \
   gvrender_core_pov.c \
   gvrender_core_pic.c \
   gvloadimage_core.c \
   ps.txt


CC_OPT += -DHAVE_CONFIG_H

vpath %.c $(GRAPHVIZ_DIR)/core

#SHARED_LIB = yes
