/*
 * \brief   Classes for records stored in core tracing buffer
 * \author  Johannes Schlatow
 * \date    2016-07-25
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef __INCLUDE__CORE_TRACE__RECORD_H_
#define __INCLUDE__CORE_TRACE__RECORD_H_

#include <util/string.h>

/* base includes */
#include <trace/timestamp.h>

namespace Kernel { namespace Trace {

	class Record;

} }

struct Kernel::Trace::Record
{
	Genode::Trace::Timestamp timestamp;
	char name[0];

	Record(Genode::Trace::Timestamp _timestamp, const char* _name, Genode::size_t namelen)
		: timestamp(_timestamp)
	{
		Genode::strncpy(name, _name, namelen+1);
	}

	inline
	void *operator new(Genode::size_t, void *p)
	{
		return p;
	}
};


#endif /* __INCLUDE__CORE_TRACE__RECORD_H_ */
