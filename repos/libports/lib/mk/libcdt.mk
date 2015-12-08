GRAPHVIZ_DIR = $(call select_from_ports,graphviz)/src/lib
LIBS    += libc 
INC_DIR += $(GRAPHVIZ_DIR)/cdt
INC_DIR += $(GRAPHVIZ_DIR)/../
INC_DIR += $(REP_DIR)/src/lib/graphviz
SRC_C   = dtclose.c dtdisc.c dtextract.c dtflatten.c \
			 dthash.c dtlist.c dtmethod.c dtopen.c dtrenew.c dtrestore.c dtsize.c \
			 dtstat.c dtstrhash.c dttree.c dttreeset.c dtview.c dtwalk.c

CC_OPT += -DHAVE_CONFIG_H

vpath %.c $(GRAPHVIZ_DIR)/cdt

#SHARED_LIB = yes
