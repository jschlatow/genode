


#ifndef _CORE__INCLUDE__TRACE__CORE_TRACING_UNIT_H_
#define _CORE__INCLUDE__TRACE__CORE_TRACING_UNIT_H_


#include <trace/source_registry.h>
#include <trace/control_area.h>

#include <dataspace/client.h>

/* base-tracing includes*/
#include <base/trace/logger.h>
#include <base/trace/core_policy.h>



namespace Genode { namespace Trace {

	class Core_Logger;

	class Core_tracing_unit;

	class Core_policy_module;

	struct Job_process;

	/**
	 * Return singleton instance of trace-source registry
	 */
	Core_tracing_unit &core_tracing_unit();
} }




struct Genode::Trace::Core_Logger : Genode::Trace::Logger_base
{

	private:

		Core_policy_module  *policy_module;

		bool _evaluate_control();


	public:

		Core_Logger();	

		bool _inhibit_core_tracing;

		void init(Trace::Control *core_control)
		{
			control = core_control;
		}

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

class Genode::Trace::Core_tracing_unit : public Trace::Source::Info_accessor
{

	private:


		/* the logger which references the "core-tracing-buffer" to trace */
		Trace::Core_Logger _trace_logger { };

		/* Source to insert into the Source_registry */
		Trace::Source _trace_source;

		/* The trace control block*/
		Trace::Control _trace_control { };

		/* ----- Info_accesor return variables ----- */
		Session_label const _session_label = "core";

		/* From the base-thread of core */
		Trace::Thread_name _name { }; 

		/* from core_cpu */
		Affinity::Location _location { };

		/* always 0 by now */
		unsigned long long _execution_time { 0 };

	public:

		/**
		 * Constructor
		 */
		Core_tracing_unit();


		/* To implement the Trace::Source::Info_accessor */
		Trace::Source::Info trace_source_info() const
		{
			return { _session_label, _name,
			        0,
			        _location };
		}

		Dataspace_capability trace_buffer()
		{
			return _trace_source.buffer();
		}


		Dataspace_capability trace_policy()
		{
			return _trace_source.policy();
		}

		size_t buffer_size()
		{
			return _trace_source.size();
		}

		Genode::Trace::Core_Logger *_logger();


		template <typename EVENT>
		void trace(EVENT const *event);
	
};


struct Genode::Trace::Job_process
{

	unsigned int job_id;
	unsigned int old_job_id;

	Job_process(unsigned int job_id, unsigned int old_job_id);

	size_t generate(Core_policy_module &policy, char *dst) const {
		return policy.job_process(dst, job_id, old_job_id);
	}

};


	}

};

#endif /* _CORE__INCLUDE__TRACE__CORE_TRACING_UNIT_H_ */







	










