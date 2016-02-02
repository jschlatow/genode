// Automatically generated file.
#include"trace.h"
#include"symbol.h"
#include"rational.h"
#include"prime_generator.h"
#include"gparams.h"
#include"debug.h"
void mem_initialize() {
initialize_symbols();
rational::initialize();
gparams::init();
}
void mem_finalize() {
finalize_trace();
finalize_symbols();
rational::finalize();
prime_iterator::finalize();
gparams::finalize();
finalize_debug();
}
