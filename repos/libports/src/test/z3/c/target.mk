TARGET = test-z3-c
LIBS   = libc z3
Z3     = $(call select_from_ports,z3)/src/lib/z3/examples/c
SRC_C  = test_capi.c

vpath test_capi.c $(Z3)
