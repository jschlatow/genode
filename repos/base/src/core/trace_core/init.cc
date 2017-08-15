#include <trace/trace_core.h>

namespace Genode { namespace Trace {
	void init();
} }

void Genode::Trace::init() {
	core_tracing_unit();
}
