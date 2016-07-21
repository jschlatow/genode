/*
 * \brief   Classes for kernel tracing
 * \author  Johannes Schlatow
 * \date    2016-07-20
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* core includes */
#include <kernel/trace.h>

/* base-internal includes */
#include <base/internal/unmanaged_singleton.h>

namespace Kernel { namespace Trace {

	enum {
		MAX_EVENT_SIZE = 64
	};

	Logger * trace_logger() { return unmanaged_singleton<Logger, 4096>(MAX_EVENT_SIZE); }

} }

Kernel::Trace::Logger::Logger(size_t max_event_size) : _max_event_size(max_event_size)
{
	_buffer()->init(BUFFER_SIZE);
}
