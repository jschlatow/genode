/*
 * \brief  Replacement of Timer::One_shot_timeout with lazy rescheduling
 * \author Johannes Schlatow
 * \date   2022-07-01
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _TIMEOUT_H_
#define _TIMEOUT_H_

/* Genode includes */
#include <timer_session/connection.h>


namespace Net {
	template <typename>
	class One_shot_timeout;
}

template <typename HANDLER>
class Net::One_shot_timeout : private Genode::Noncopyable,
                              private Genode::Timeout_handler
{
	private:

		using Duration          = Genode::Duration;
		using Timeout           = Genode::Timeout;
		using Microseconds      = Genode::Microseconds;

		typedef void (HANDLER::*Handler_method)(Duration);

		Timer::Connection    &_timer;
		Timeout               _timeout;
		HANDLER              &_object;
		Handler_method const  _method;
		Microseconds          _deadline_wanted { 0 };


		/*********************
		 ** Timeout_handler **
		 *********************/

		void handle_timeout(Duration curr_time) override
		{
			if (_deadline_wanted.value > 0) {
				Microseconds curr_time_us = curr_time.trunc_to_plain_us();
				if (curr_time_us.value < _deadline_wanted.value) {
					/* reschedule if _deadline_wanted is != 0 and in the future */
					schedule(Microseconds { _deadline_wanted.value - curr_time_us.value });
					_deadline_wanted.value = 0;
					return;
				}
			}

			(_object.*_method)(curr_time);
		}

	public:

		One_shot_timeout(Timer::Connection &timer,
		                 HANDLER           &object,
		                 Handler_method     method)
		:
			_timer   { timer },
			_timeout { timer },
			_object  { object },
			_method  { method }
		{ }

		void schedule(Microseconds duration) {
			_timeout.schedule_one_shot(duration, *this); }

		/**
		 * Variant of schedule() that does not replace the currently scheduled
		 * timeout if it is at least 'min_duration' in the future.
		 * If the new deadline is at most 'accuracy' microseconds after the
		 * current deadline, we just ignore the update.
		 */
		void schedule_lazy(Microseconds duration, Microseconds min_duration, Microseconds accuracy = Microseconds { 10 })
		{
			if (!_timeout.scheduled())
				schedule(duration);

			using uint64_t = Genode::uint64_t;

			uint64_t const curr_time_us {
				_timer.curr_time().trunc_to_plain_us().value };

			uint64_t const deadline_us {
				duration.value <= ~(uint64_t)0 - curr_time_us ?
					curr_time_us + duration.value : ~(uint64_t)0 };

			/* scheduled timeout is at least min_duration ahead */
			if (deadline_us > _timeout.deadline().value &&
			    curr_time_us + min_duration.value <= _timeout.deadline().value) {
				/* do nothing if wanted deadline will be too close */
				if (_timeout.deadline().value + accuracy.value < deadline_us) {
					_deadline_wanted.value = deadline_us;
				}
			} else {
				schedule(duration);
			}
		}

		void discard() { _timeout.discard(); }

		bool scheduled() { return _timeout.scheduled(); }
};

#endif /* _TIMEOUT_H_ */
