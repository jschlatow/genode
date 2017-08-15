/*
 * \brief  Event tracing infrastructure
 * \author Norman Feske
 * \date   2013-08-09
 */

/*
 * Copyright (C) 2013-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__BASE__TRACE__CORE_POLICY_H_
#define _INCLUDE__BASE__TRACE__CORE_POLICY_H_

#include <base/stdint.h>

namespace Genode {

	class Msgbuf_base;
	class Signal_context;
	class Rpc_object_base;

	namespace Trace { class Core_policy_module; }
}


/**
 * Header of tracing policy
 */
struct Genode::Trace::Core_policy_module
{
	size_t (*max_event_size)  ();

	/* Remember the last scheduled thread */
	unsigned int _last_thread_address;
	size_t (*job_process)     (char *, unsigned int, unsigned int);

};

#endif /* _INCLUDE__BASE__TRACE__CORE_POLICY_H_ */
