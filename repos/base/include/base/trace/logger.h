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

#ifndef _INCLUDE__BASE__TRACE__LOGGER_H_
#define _INCLUDE__BASE__TRACE__LOGGER_H_

#include <base/trace/buffer.h>
#include <cpu_session/cpu_session.h>

namespace Genode { namespace Trace {

	class Control;
	class Policy_module;
	class Logger_base;
	class Logger;
	class Core_Logger;
} }




struct Genode::Trace::Logger_base
{
	
	protected:

		Control    *control         { nullptr };
		bool        enabled         { false };
		unsigned    policy_version  { 0 };
		Buffer     *buffer          { nullptr };
		size_t      max_event_size  { 0 };

	public:

		Logger_base() { }

		bool initialized() { return control != 0; }

};



/**
 * Facility for logging events to a thread-specific buffer
 */
struct Genode::Trace::Logger : Genode::Trace::Logger_base
{
	private:

		Policy_module     *policy_module { nullptr };
		Thread_capability  thread_cap    { };
		Cpu_session       *cpu           { nullptr };
		bool               pending_init  { false };
		bool _evaluate_control();

		/*
		 * Noncopyable
		 */
		Logger(Logger const &);
		Logger &operator = (Logger const &);

	public:

		Logger();

		bool init_pending() { return pending_init; }

		void init_pending(bool val) { pending_init = val; }

		void init(Thread_capability, Cpu_session*, Control*);

		/**
		 * Log binary data to trace buffer
		 */
		__attribute__((optimize("-fno-delete-null-pointer-checks")))
		void log(char const *msg, size_t len)
		{
			
			if (!this || !_evaluate_control()) return;

			memcpy(buffer->reserve(len), msg, len);
			buffer->commit(len);
		}



		/**
		 * Log event to trace buffer
		 */
		template <typename EVENT>
		__attribute__((optimize("-fno-delete-null-pointer-checks")))
		void log(EVENT const *event)
		{

			if (!this || !_evaluate_control()) return;
			
			buffer->commit(event->generate(*policy_module, buffer->reserve(max_event_size)));
		}

};

#endif /* _INCLUDE__BASE__TRACE__LOGGER_H_ */
