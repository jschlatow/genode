SQLITE     = sqlite-amalgamation-3080403
SQLITE_DIR = $(call select_from_ports,sqlite3)/src/lib/sqlite/$(SQLITE)/
LIBS    += libc
INC_DIR += $(SQLITE_DIR)
SRC_C    = 	sqlite3.c
CC_OPT  += -DSQLITE_THREADSAFE=0

# set minimum file descriptor
CC_OPT  += -DSQLITE_MINIMUM_FILE_DESCRIPTOR=0

# workaround issue #754, i.e. malloc() should return 8-byte-aligned pointers
CC_OPT  += -DSQLITE_4_BYTE_ALIGNED_MALLOC

# not sync
CC_OPT  += -DSQLITE_NO_SYNC

# debug
#CC_OPT  += -DSQLITE_DEBUG

vpath %.c $(SQLITE_DIR)

#SHARED_LIB = yes
