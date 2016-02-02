TARGET = test-z3-c++
LIBS   = stdcxx z3
Z3     = $(call select_from_ports,z3)/src/lib/z3/examples/c++
SRC_CC = example.cpp

vpath example.cpp $(Z3)
