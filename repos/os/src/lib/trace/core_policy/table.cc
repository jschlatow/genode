/*
 * \brief  Header of tracing policy module
 * \author Norman Feske
 * \date   2013-08-15
 */

/*
 * Copyright (C) 2013-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <trace/core_policy.h>
#include <base/trace/core_policy.h>

extern "C" {

	Genode::Trace::Core_policy_module policy_jump_table =
	{
		max_event_size,
		job_process
	};
}
