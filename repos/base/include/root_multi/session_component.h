/*
 * \brief  Base session component for Root_component_multi
 * \author Johannes Schlatow
 * \date   2014-07-03
 *
 * This session component creates its own entrypoint.
 */

#pragma once

/* Genode includes */
#include <base/rpc.h>

namespace Genode
{
	template <typename SESSION_TYPE, unsigned STACK_SIZE>
	class Session_component_multi : public Genode::Rpc_object<SESSION_TYPE>
	{
		private:

			Genode::Rpc_entrypoint  _entrypoint;
			Session_capability      _session_cap;

		public:

			/**
			 * Constructor
			 */
			Session_component_multi(Genode::Cap_session *cap, const char *label)
			:
				_entrypoint(cap, STACK_SIZE, label),
				_session_cap(_entrypoint.manage(this))
			{ }

			/**
			 * Destructor
			 */
			~Session_component_multi()
			{
				_entrypoint.dissolve(this);
			}

			/**
			 * Return true if capability belongs to session object
			 */
			bool belongs_to(Genode::Session_capability cap)
			{
				return (Session_component_multi*)_entrypoint.first()->find_by_obj_id(cap.local_name()) == this;
			}

			/**
			 * Return session capability
			 */
			Session_capability cap() { return _session_cap; }
	};
}
