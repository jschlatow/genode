/*
 * \brief  Test for core tracing
 * \author Johannes Schlatow
 * \date   2016-07-21
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <trace_session/connection.h>
#include <timer_session/connection.h>

#include <base/log.h>
#include <base/component.h>

#include <core_trace/record.h>

using namespace Genode;
using Kernel::Trace::Record;

class Trace_buffer_monitor
{
	private:

		enum { MAX_ENTRY_BUF = 256 };
		char                  _buf[MAX_ENTRY_BUF];

		Trace::Buffer        *_buffer;
		Trace::Buffer::Entry _curr_entry;

		const char *_terminate_entry(Trace::Buffer::Entry const &entry)
		{
			size_t len = min(entry.length() + 1, MAX_ENTRY_BUF);
			memcpy(_buf, entry.data(), len);
			_buf[len-1] = '\0';

			return _buf;
		}

	public:

		Trace_buffer_monitor(Dataspace_capability ds_cap)
		:
			_buffer(env()->rm_session()->attach(ds_cap)),
			_curr_entry(_buffer->first())
		{ }

		~Trace_buffer_monitor()
		{
			if (_buffer)
				env()->rm_session()->detach(_buffer);
		}

		void dump()
		{
			Genode::log("overflows: ", _buffer->wrapped());

			Genode::log("read all remaining events");
			for (; !_curr_entry.last(); _curr_entry = _buffer->next(_curr_entry)) {
				/* omit empty entries */
				if (_curr_entry.length() == 0)
					continue;

				Record *record = (Record*)_terminate_entry(_curr_entry);
				if (record)
					Genode::log(record->timestamp, " ", (const char*)record->name);
			}

			/* reset after we read all available entries */
			_curr_entry = _buffer->first();
		}
};

namespace Component
{
	Genode::size_t stack_size() { return 2 * 1024 * sizeof(long); }

	void construct(Genode::Env &env) {

		Genode::log("--- test-core_trace started ---");

		static Genode::Trace::Connection trace(1024*1024, 64*1024, 0);
		static Timer::Connection timer;

		static Dataspace_capability ds_cap = trace.core_buffer();

		if (!ds_cap.valid()) {
			Genode::error("invalid capability");
		}

		static Trace_buffer_monitor test_monitor(ds_cap);

		for (size_t cnt = 0; cnt < 5; cnt++) {
			timer.msleep(3000);
			test_monitor.dump();
		}

		Genode::log("--- test-core_trace finished ---");
	}
}
