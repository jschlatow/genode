/*
 * \brief  Example for a server with multiple entrypoints
 * \author Johannes Schlatow
 * \date   2014-07-03
 */

/* Genode includes */
#include <base/env.h>
#include <base/sleep.h>
#include <base/heap.h>
#include <cap_session/connection.h>

#include <log_session/log_session.h>
#include <root_multi/component.h>
#include <root_multi/session_component.h>

#include <base/printf.h>

using namespace Genode;

namespace Log
{
	enum { STACK_SIZE = 4096 };
	
    /**
	 * Log session with own entrypoint
	 */
	class Session_component : public Genode::Session_component_multi<Log_session, STACK_SIZE>,
	                          public Genode::List<Session_component>::Element
	{
		public:
			Session_component(Genode::Cap_session *cap)
			: Genode::Session_component_multi<Log_session, STACK_SIZE>(cap, "log_session_ep")
			{ }

			/***************************
			 ** Log session interface **
			 ***************************/
			size_t write(String const &string)
			{
				Genode::printf(string.string());

				/* block indefinitely for testing */
				sleep_forever();

				return sizeof(string.string());
			}

	};

	class Root_component : public Genode::Root_component_multi<Session_component>
	{
		private:
			Session_component *_create_session(const char *args)
			{
				/* create new session for the requested file */
				return new (md_alloc()) Session_component(cap_session());
			}
		public:

			/**
			 * Constructor
			 *
			 * \param  entrypoint  entrypoint to be used for the sessions
			 * \param  md_alloc    meta-data allocator used for the sessions
			 * \param  cap         cap session
			 */
			Root_component(Genode::Rpc_entrypoint  &entrypoint,
						      Genode::Allocator       &md_alloc,
						      Genode::Cap_session     &cap)
			:
				Genode::Root_component_multi<Session_component>(&entrypoint, &md_alloc, &cap)
			{ }
	};
}

int main()
{
	enum { STACK_SIZE = 4096 };

	/*
	 * Initialize server entry point that serves the root interface.
	 */
	static Cap_connection cap;
	static Rpc_entrypoint ep(&cap, STACK_SIZE, "log_ep");

	/*
	 * Use sliced heap to allocate each session component at a separate
	 * dataspace.
	 */
	static Sliced_heap sliced_heap(env()->ram_session(), env()->rm_session());

	/*
	 * Create root interface for service
	 */
	static Log::Root_component multi_root(ep, sliced_heap, cap);

	/*
	 * Announce service at our parent.
	 */
	env()->parent()->announce(ep.manage(&multi_root));

	PINF("Server started");

	sleep_forever();
	return 0;
}
