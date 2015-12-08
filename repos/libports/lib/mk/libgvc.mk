GRAPHVIZ_DIR = $(call select_from_ports,graphviz)/src/lib
LIBS    += libc expat libgvcommon
INC_DIR += $(GRAPHVIZ_DIR)/gvc
INC_DIR += $(GRAPHVIZ_DIR)/cgraph
INC_DIR += $(GRAPHVIZ_DIR)/common
INC_DIR += $(GRAPHVIZ_DIR)/pathplan
INC_DIR += $(GRAPHVIZ_DIR)/cdt
INC_DIR += $(GRAPHVIZ_DIR)/../
INC_DIR += $(REP_DIR)/src/lib/graphviz
SRC_C   = gvrender.c gvlayout.c gvdevice.c gvloadimage.c \
			 gvcontext.c gvjobs.c gvevent.c gvplugin.c gvconfig.c \
			 gvtextlayout.c gvusershape.c gvbuffstderr.c gvc.c

CC_OPT += -DHAVE_CONFIG_H

vpath %.c $(GRAPHVIZ_DIR)/gvc

#SHARED_LIB = yes
