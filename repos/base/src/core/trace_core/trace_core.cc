#include <trace/trace_core.h>



/* Core_Logger-Methods */

using namespace Genode;

Genode::Trace::Core_Logger::Core_Logger() 
		: Genode::Trace::Logger_base::Logger_base(), 
		policy_module(0),
		_inhibit_core_tracing(true) {}


bool Genode::Trace::Core_Logger::_evaluate_control()
{
	
	/* check process-global and thread-specific tracing condition */
	if (_inhibit_core_tracing || !control || control->tracing_inhibited())
		return false;

	if (control->state_changed()) {

		/* suppress tracing during initialization */
		Control::Inhibit_guard guard(*control);

		if (control->to_be_disabled()) {
			
			/* unload policy */
			if (policy_module) {
				env_deprecated()->rm_session()->detach(policy_module);
				policy_module = 0;
			}

			/* unmap trace buffer */
			if (buffer) {
				env_deprecated()->rm_session()->detach(buffer);
				buffer = 0;
			}

			/* inhibit generation of trace events */
			enabled = false;
			control->acknowledge_disabled();



		} else if (control->to_be_enabled()) {
			control->acknowledge_enabled();
			enabled = true;
		}

	}


	if (enabled && (policy_version != control->policy_version())) {
	
		/* suppress tracing during policy change */
		Control::Inhibit_guard guard(*control);

		/* obtain policy */
		try {

			Dataspace_capability policy_ds = Genode::Trace::core_tracing_unit().trace_policy();
			Dataspace_capability buffer_ds = Genode::Trace::core_tracing_unit().trace_buffer();
			size_t buffer_size             = Genode::Trace::core_tracing_unit().buffer_size();

			if (!policy_ds.valid()) {
				warning("could not obtain trace policy");
				control->error();
				enabled = false;
				return false;
			}

			if (!buffer_ds.valid()) {
				warning("could not obtain trace buffer");
				control->error();
				enabled = false;
				return false;
			}


			max_event_size = 0;
			policy_module  = 0;

			buffer = 0;

			buffer = env_deprecated()->rm_session()->attach(buffer_ds);
			policy_module = env_deprecated()->rm_session()-> attach_non_local_address_executable(policy_ds);

			/* -------------------- policy part -------------------- */

			/* relocate function pointers of policy callback table */
			for (unsigned i = 0; i < sizeof(Trace::Core_policy_module)/sizeof(void *); i++) {

				((addr_t *)policy_module)[i] += (addr_t)(policy_module);

			}

			max_event_size = policy_module->max_event_size();
		

			/* -------------------- buffer part -------------------- */

			buffer->init(buffer_size); 

		} catch (...) { }

		policy_version = control->policy_version();

	}

	return enabled && policy_module;
}





/* Event-Methods */

Genode::Trace::Job_process::Job_process(unsigned int job_id, unsigned int old_job_id) : job_id(job_id), old_job_id(old_job_id)
{
	Genode::Trace::core_tracing_unit().trace(this);
}




/* Core_tracing_unit-Methods */


Genode::Trace::Core_tracing_unit::Core_tracing_unit() 
	:  _trace_source(*this, _trace_control),
	   _location(0,0)
{
	_trace_logger._inhibit_core_tracing = false;

	/* Info_Accessor values */
	_execution_time = 0;
	_name = "coreThread";

	/* Reset the control block */
	_trace_control.reset();	

	/* Insert the source into the Source_registry */
	Trace::sources().insert(&_trace_source);
}


template <typename EVENT>
void Genode::Trace::Core_tracing_unit::trace(EVENT const *event) {
	 _logger()->log(event);
}


Genode::Trace::Core_Logger* Genode::Trace::Core_tracing_unit::_logger(){
			

	if(_trace_logger.initialized()){
		return &_trace_logger;	
	}

	_trace_logger.init(&_trace_control);

	return &_trace_logger;

}

Genode::Trace::Core_tracing_unit &Trace::core_tracing_unit()
{
	static Trace::Core_tracing_unit inst;
	return inst;
}






