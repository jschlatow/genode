GRAPHVIZ_DIR = $(call select_from_ports,graphviz)/src/lib
LIBS    += libc libcdt
INC_DIR += $(GRAPHVIZ_DIR)/cgraph
INC_DIR += $(REP_DIR)/src/lib/graphviz
SRC_C   = agerror.c agxbuf.c apply.c attr.c edge.c \
			 flatten.c graph.c grammar.y id.c imap.c io.c mem.c node.c \
			 obj.c pend.c rec.c refstr.c subg.c utils.c write.c scan.c

CC_OPT += -DHAVE_CONFIG_H

vpath %.c $(GRAPHVIZ_DIR)/cgraph

#SHARED_LIB = yes
