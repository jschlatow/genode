GRAPHVIZ_DIR = $(call select_from_ports,graphviz)/src/lib
LIBS    += libc
INC_DIR += $(GRAPHVIZ_DIR)/gvc
INC_DIR += $(GRAPHVIZ_DIR)/common
INC_DIR += $(GRAPHVIZ_DIR)/cgraph
INC_DIR += $(GRAPHVIZ_DIR)/cdt
INC_DIR += $(GRAPHVIZ_DIR)/label
INC_DIR += $(GRAPHVIZ_DIR)/../
INC_DIR += $(REP_DIR)/src/lib/graphviz
SRC_C = xlabels.c index.c node.c rectangle.c split.q.c

CC_OPT += -DHAVE_CONFIG_H

vpath %.c $(GRAPHVIZ_DIR)/label

#SHARED_LIB = yes
