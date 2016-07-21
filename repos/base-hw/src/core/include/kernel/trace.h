/*
 * \brief   Classes for kernel tracing
 * \author  Johannes Schlatow
 * \date    2016-07-22
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _CORE__INCLUDE__KERNEL__TRACE_H_
#define _CORE__INCLUDE__KERNEL__TRACE_H_

#include <core_trace/record.h>
#include <base/trace/buffer.h>
#include <dataspace_component.h>

/* base includes */
#include <trace/timestamp.h>

namespace Kernel { namespace Trace {
	using namespace Genode;
	using Genode::Trace::Buffer;

	class Logger;
	class Event;

	Logger * trace_logger();
} }

class Kernel::Trace::Logger
{
	private:
		enum {
			/* must be rounded to page size */
			BUFFER_SIZE = 4096
		};

		char                  _data[BUFFER_SIZE];
		Dataspace_component   _ds = {BUFFER_SIZE, (addr_t)_data, CACHED, false,  0};

		Buffer *_buffer() { return (Buffer*)_data; }

		size_t  _max_event_size;

	public:

		/**
		 * Constructor
		 */
		Logger() { }

		Logger(size_t max_event_size);

		/**
		 * Log binary data to trace buffer
		 */
		void log(char const *msg, size_t len)
		{
			if (!this) return;

			memcpy(_buffer()->reserve(len), msg, len);
			_buffer()->commit(len);
		}

		/**
		 * Log event to trace buffer
		 */
		template <typename EVENT>
		void log(EVENT const *event)
		{
			if (!this) return;

			_buffer()->commit(event->generate(_buffer()->reserve(_max_event_size)));
		}

		Dataspace_component *dataspace()
		{
			if (!this) return 0;

			return &_ds;
		}
};

struct Kernel::Trace::Event
{
	char        const *name;

	Event(char const *name)
	: name(name)
	{
		trace_logger()->log(this);
	}

	size_t generate(char *dst) const {
		size_t namelen = strlen(name);
		size_t len = namelen + sizeof(Record) + 1;

		new (dst) Record(Genode::Trace::timestamp(), name, namelen);

		return len;
	}
};

#endif /* _CORE__INCLUDE__KERNEL__TRACE_H_ */

