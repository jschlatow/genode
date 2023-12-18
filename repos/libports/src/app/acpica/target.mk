TARGET   := acpica
SRC_CC   := os.cc printf.cc report.cc
REQUIRES := x86
LIBS     += base acpica format

CC_OPT += -DACPI_DEBUGGER
#CC_OPT += -DACPI_DEBUG_OUTPUT

CC_CXX_WARN_STRICT =
