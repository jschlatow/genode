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


#endif /* _INCLUDE__TRACE__PROBE_H_ */
