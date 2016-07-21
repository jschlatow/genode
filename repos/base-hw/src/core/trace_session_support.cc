/*
 * \brief   Platform specific parts of TRACE session
 * \author  Johannes Schlatow
 * \date    2016-07-21
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <dataspace/capability.h>

/* core includes */
#include <trace/session_component.h>
#include <kernel/trace.h>

#include <core_env.h>

using namespace Genode;

namespace Kernel { namespace Trace {

	struct Core_buffer 
	{
		Dataspace_capability ds_cap;

		Core_buffer(Dataspace_component *ds) {
			if (ds)
				ds_cap = core_env()->entrypoint()->manage(ds);
		}
	};

} }

Dataspace_capability Genode::Trace::Session_component::core_buffer()
{
	static Kernel::Trace::Core_buffer core_buffer(Kernel::Trace::trace_logger()->dataspace());

	return core_buffer.ds_cap;
}
