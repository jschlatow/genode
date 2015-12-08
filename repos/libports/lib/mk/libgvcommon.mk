GRAPHVIZ_DIR = $(call select_from_ports,graphviz)/src/lib
LIBS    += libc libm expat
INC_DIR += $(GRAPHVIZ_DIR)/gvc
INC_DIR += $(GRAPHVIZ_DIR)/common
INC_DIR += $(GRAPHVIZ_DIR)/fdpgen
INC_DIR += $(GRAPHVIZ_DIR)/pathplan
INC_DIR += $(GRAPHVIZ_DIR)/cgraph
INC_DIR += $(GRAPHVIZ_DIR)/cdt
INC_DIR += $(GRAPHVIZ_DIR)/xdot
INC_DIR += $(GRAPHVIZ_DIR)/label
INC_DIR += $(GRAPHVIZ_DIR)/../
INC_DIR += $(REP_DIR)/src/lib/graphviz
#SRC_C   =    memory.c textspan.c emit.c \
#			 input.c ../xdot/xdot.c globals.c labels.c utils.c \
#			 shapes.c geom.c arrows.c colxlate.c \
#			 taper.c htmltable.c splines.c intset.c pointset.c \
#			 htmlparse.c htmllex.c ns.c psusershape.c timing.c \
#			 ellipse.c ../pathplan/util.c 
SRC_C = arrows.c colxlate.c ellipse.c textspan.c \
   args.c memory.c globals.c htmllex.c htmlparse.y htmltable.c input.c \
   pointset.c intset.c postproc.c routespl.c splines.c psusershape.c \
   timing.c labels.c ns.c shapes.c utils.c geom.c taper.c \
   output.c emit.c ../xdot/xdot.c

CC_OPT += -DHAVE_CONFIG_H

vpath %.c $(GRAPHVIZ_DIR)/common

#SHARED_LIB = yes
