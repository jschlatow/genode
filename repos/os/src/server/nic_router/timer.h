/*
 * \brief  Replacement of Timer::Connection
 * \author Johannes Schlatow
 * \date   2022-07-07
 *
 * This implementation prevents frequent calls of elapsed_us() on ARM.
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SAMPLED_TIMER_H_
#define _SAMPLED_TIMER_H_

/* Genode includes */
#include <timer_session/connection.h>


namespace Net {
	class Cached_timer;
}

class Net::Cached_timer : public ::Timer::Connection
{
	private:

		using Duration = Genode::Duration;
		using Microseconds = Genode::Microseconds;

		Duration _cached_time { Microseconds { 0 } };

	public:

		Cached_timer (Genode::Env &env)
		: Timer::Connection(env)
		{ }

		/* update curr_time from timer */
		void update_time() {
			_cached_time = Timer::Connection::curr_time(); }

		/* set curr_time from argument */
		void curr_time(Duration curr_time) { _cached_time = curr_time; }

		/* update curr_time and return */
		Duration curr_time() override
		{
			update_time();
			return cached_time();
		}

		/* return last known curr_time without updating */
		Duration cached_time() { return _cached_time; }
};

#endif /* _SAMPLED_TIMER_H_ */
