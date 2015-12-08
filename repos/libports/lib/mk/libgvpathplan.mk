GRAPHVIZ_DIR = $(call select_from_ports,graphviz)/src/lib
LIBS    += libc
INC_DIR += $(GRAPHVIZ_DIR)/gvc
INC_DIR += $(GRAPHVIZ_DIR)/common
INC_DIR += $(GRAPHVIZ_DIR)/cgraph
INC_DIR += $(GRAPHVIZ_DIR)/cdt
INC_DIR += $(GRAPHVIZ_DIR)/pathplan
INC_DIR += $(GRAPHVIZ_DIR)/../
INC_DIR += $(REP_DIR)/src/lib/graphviz
SRC_C = cvt.c inpoly.c route.c shortest.c \
   shortestpth.c solvers.c triang.c util.c visibility.c

CC_OPT += -DHAVE_CONFIG_H

vpath %.c $(GRAPHVIZ_DIR)/pathplan

#SHARED_LIB = yes
