SRC_C += utcb.c
SRC_S += atomic_ops.S

vpath atomic_ops.S $(L4_BUILD_DIR)/source/pkg/l4sys/lib/src/ARCH-sparc
vpath utcb.c         $(L4_BUILD_DIR)/source/pkg/l4sys/lib/src
