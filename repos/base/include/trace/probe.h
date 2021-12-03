/*
 * \brief  Trace probes
 * \author Johannes Schlatow
 * \date   2021-12-01
 *
 * Convenience macros for creating user-defined trace checkpoints.
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__TRACE__PROBE_H_
#define _INCLUDE__TRACE__PROBE_H_

#include <base/trace/events.h>

namespace Genode { namespace Trace {

	class Duration
	{
		private:
			char               const *_end_name;
			unsigned long long const  _data;

			Duration(Duration const &) = delete;

			Duration & operator = (Duration const &) = delete;

		public:
			Duration(char const * start_name, char const * end_name, unsigned long long data)
			: _end_name(end_name), _data(data)
			{ Checkpoint(start_name, data); }

			~Duration()
			{ Checkpoint(_end_name, _data); }
	};

} }


/**
 * Trace a single checkpoint named after the current function.
 *
 * The argument 'data' specifies the payload as an unsigned value.
 */
#define GENODE_TRACE_CHECKPOINT(data) \
	Genode::Trace::Checkpoint(__PRETTY_FUNCTION__, (unsigned long long)data);


/**
 * Variant of 'GENODE_TRACE_CHECKPOINT' that accepts the name of the checkpoint as argument.
 *
 * The argument 'data' specifies the payload as an unsigned value.
 * The argument 'name' specifies the name of the checkpoint.
 */
#define GENODE_TRACE_CHECKPOINT_NAMED(data, name) \
	Genode::Trace::Checkpoint(#name, (unsigned long long)data);


/**
 * Trace a pair of checkpoints when entering and leaving the current scope.
 *
 * The argument 'data' specifies the payload as an unsigned value.
 * The argument 'name' specifies the names of the checkpoint suffixed with
 * '_start' resp. '_end'.
 */
#define GENODE_TRACE_DURATION_NAMED(data, name) \
	Genode::Trace::Duration(#name"_start", #name"_end", (unsigned long long)data);


#endif /* _INCLUDE__TRACE__PROBE_H_ */
