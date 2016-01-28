#
# Z3 solver library
#

include $(REP_DIR)/lib/mk/z3.inc

# polynomial files
SRC_CC   = algebraic_numbers.cpp \
	polynomial_cache.cpp \
	polynomial.cpp \
	polynomial_factorization.cpp \
	rpolynomial.cpp \
	sexpr2upolynomial.cpp \
	upolynomial.cpp \
	upolynomial_factorization.cpp

LIBS     = stdcxx z3-util gmp

vpath %.cpp  $(Z3_DIR)/src/math/polynomial

#SHARED_LIB = no
