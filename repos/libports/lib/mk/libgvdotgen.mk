GRAPHVIZ_DIR = $(call select_from_ports,graphviz)/src/lib
LIBS    += libc 
INC_DIR += $(GRAPHVIZ_DIR)/gvc
INC_DIR += $(GRAPHVIZ_DIR)/common
INC_DIR += $(GRAPHVIZ_DIR)/pathplan
INC_DIR += $(GRAPHVIZ_DIR)/cgraph
INC_DIR += $(GRAPHVIZ_DIR)/cdt
INC_DIR += $(GRAPHVIZ_DIR)/pack
INC_DIR += $(GRAPHVIZ_DIR)/../
INC_DIR += $(REP_DIR)/src/lib/graphviz
SRC_C = acyclic.c class1.c class2.c cluster.c compound.c \
   conc.c decomp.c fastgr.c flat.c dotinit.c mincross.c \
   position.c rank.c sameport.c dotsplines.c aspect.c

CC_OPT += -DHAVE_CONFIG_H

vpath %.c $(GRAPHVIZ_DIR)/dotgen

#SHARED_LIB = yes
