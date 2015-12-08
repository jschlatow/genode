/*
 * \brief  Root component that creates an entrypoint for each session
 * \author Johannes Schlatow
 * \date   2014-07-03
 */

#pragma once

#include <util/arg_string.h>
#include <base/rpc_server.h>
#include <root/root.h>

namespace Genode {

	template <typename SESSION_TYPE>
	class Root_component_multi : public Genode::Rpc_object<Genode::Typed_root<SESSION_TYPE> >
	{
		private:

			Genode::Allocator              *_md_alloc;
			Genode::Cap_session            *_cap_session;
			Genode::Lock                    _sessions_lock;
			Genode::List<SESSION_TYPE>      _sessions;

		protected:

			/**
			 * Create new session (to be implemented by a derived class)
			 *
			 * Only a derived class knows the constructor arguments of
			 * a specific session. Therefore, we cannot unify the call
			 * of its 'new' operator and must implement the session
			 * creation at a place, where the required knowledge exist.
			 *
			 * In the implementation of this function, the heap, provided
			 * by 'Root_component_multi' must be used for allocating the session
			 * object.
			 *
			 * If the server implementation does not evaluate the session
			 * affinity, it suffices to override the overload without the
			 * affinity argument.
			 *
			 * \throw Allocator::Out_of_memory  typically caused by the
			 *                                  meta-data allocator
			 * \throw Root::Invalid_args        typically caused by the
			 *                                  session-component constructor
			 */
			virtual SESSION_TYPE *_create_session(const char *args,
			                                      Affinity const &)
			{
				return _create_session(args);
			}

			virtual SESSION_TYPE *_create_session(const char *args)
			{
				throw Root::Invalid_args();
			}

			/**
			 * Inform session about a quota upgrade
			 *
			 * Once a session is created, its client can successively extend
			 * its quota donation via the 'Parent::transfer_quota' function.
			 * This will result in the invokation of 'Root::upgrade' at the
			 * root interface the session was created with. The root interface,
			 * in turn, informs the session about the new resources via the
			 * '_upgrade_session' function. The default implementation is
			 * suited for sessions that use a static amount of resources
			 * accounted for at session-creation time. For such sessions, an
			 * upgrade is not useful. However, sessions that dynamically
			 * allocate resources on behalf of its client, should respond to
			 * quota upgrades by implementing this function.
			 *
			 * \param session  session to upgrade
			 * \param args     description of additional resources in the
			 *                 same format as used at session creation
			 */
			virtual void _upgrade_session(SESSION_TYPE *, const char *) { }

			virtual void _destroy_session(SESSION_TYPE *session) {
				destroy(_md_alloc, session); }

			Genode::Allocator     *md_alloc()    { return _md_alloc; }
			Genode::Cap_session   *cap_session() { return _cap_session; }

		public:

			/**
			 * Constructor
			 */
			Root_component_multi(Genode::Rpc_entrypoint *session_ep,
			                     Genode::Allocator      *md_alloc,
			                     Genode::Cap_session    *cap)
			:
				_md_alloc(md_alloc),
				_cap_session(cap)
			{ }


			/********************
			 ** Root interface **
			 ********************/

			Genode::Session_capability session(Root::Session_args const &args, Affinity const &affinity)
			{
				using namespace Genode;

				if (!args.is_valid_string()) throw Root::Invalid_args();

				/*
				 * We need to decrease 'ram_quota' by
				 * the size of the session object.
				 */
				size_t ram_quota = Arg_string::find_arg(args.string(), "ram_quota").long_value(0);
				size_t needed = sizeof(SESSION_TYPE) + md_alloc()->overhead(sizeof(SESSION_TYPE));

				if (needed > ram_quota) {
					PERR("Insufficient ram quota, provided=%zu, required=%zu",
					     ram_quota, needed);
					throw Root::Quota_exceeded();
				}

				size_t const remaining_ram_quota = ram_quota - needed;

				/*
				 * Deduce ram quota needed for allocating the session object from the
				 * donated ram quota.
				 *
				 * XXX  the size of the 'adjusted_args' buffer should dependent
				 *      on the message-buffer size and stack size.
				 */
				enum { MAX_ARGS_LEN = 256 };
				char adjusted_args[MAX_ARGS_LEN];
				strncpy(adjusted_args, args.string(), sizeof(adjusted_args));
				char ram_quota_buf[64];
				snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu",
				         remaining_ram_quota);
				Arg_string::set_arg(adjusted_args, sizeof(adjusted_args),
				                    "ram_quota", ram_quota_buf);

				SESSION_TYPE *s = 0;
				try { s = _create_session(adjusted_args, affinity); }
				catch (Allocator::Out_of_memory) { throw Root::Quota_exceeded(); }

				Lock::Guard guard(_sessions_lock);

				_sessions.insert(s);
				return s->cap();
			}

			void upgrade(Genode::Session_capability session, Root::Upgrade_args const &args)
			{
				if (!args.is_valid_string()) throw Root::Invalid_args();

				Genode::Lock::Guard guard(_sessions_lock);

				/*
				 * Lookup session-component by its capability by asking each
				 * entry point for object belonging to the capability. Only one
				 * entry point is expected to give the right answer.
				 */
				SESSION_TYPE *sc = _sessions.first();
				for (; sc && !sc->belongs_to(session); sc = sc->next());

				if (!sc) {
					PWRN("attempted to upgrade non-existing session");
					return;
				}

				_upgrade_session(sc, args.string());
			}


			void close(Genode::Session_capability session_cap)
			{
				Genode::Lock::Guard guard(_sessions_lock);

				/*
				 * Lookup session-component by its capability by asking each
				 * entry point for object belonging to the capability. Only one
				 * entry point is expected to give the right answer.
				 */
				SESSION_TYPE *sc = _sessions.first();
				for (; sc && !sc->belongs_to(session_cap); sc = sc->next());

				if (!sc) {
					PWRN("attempted to close non-existing session");
					return;
				}

				_sessions.remove(sc);
				_destroy_session(sc);
			}
	};
}
