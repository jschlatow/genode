/*
 * \brief  Replacement of Timer::One_shot_timeout with lazy rescheduling
 * \author Johannes Schlatow
 * \date   2022-07-01
 *
 * This implementation prevents rescheduling when a timeout is frequently
 * updated with only marginal changes. Timeouts within a certain accuracy
 * threshold of the existing timeout will be ignored. Otherwise, earlier
 * timeouts will always be rescheduled whereas later timeouts are never
 * applied immediately but only when the scheduled timeout occured.
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
	class Lazy_one_shot_timeout;
}

template <typename HANDLER>
class Net::Lazy_one_shot_timeout : private Timer::One_shot_timeout<HANDLER>
{
	private:
		using One_shot_timeout = Timer::One_shot_timeout<HANDLER>;
		using Microseconds     = typename One_shot_timeout::Microseconds;
		using Duration         = typename One_shot_timeout::Duration;
		using Handler_method   = typename One_shot_timeout::Handler_method;

		Timer::Connection    &_timer;
		Microseconds const    _accuracy;
		Microseconds          _deadline_wanted { 0 };

		/*********************
		 ** Timeout_handler **
		 *********************/

		void handle_timeout(Duration curr_time) override
		{
			if (_deadline_wanted.value > 0) {
				Microseconds curr_time_us = curr_time.trunc_to_plain_us();
				if (curr_time_us.value + _accuracy.value < _deadline_wanted.value) {
					/* reschedule if _deadline_wanted is != 0 and in the future (minus tolerance) */
					One_shot_timeout::schedule(Microseconds { _deadline_wanted.value - curr_time_us.value });
					_deadline_wanted.value = 0;
					return;
				}
			}

			One_shot_timeout::handle_timeout(curr_time);
		}

	public:

		using One_shot_timeout::discard;
		using One_shot_timeout::scheduled;
		using One_shot_timeout::deadline;

		Lazy_one_shot_timeout(Timer::Connection &timer,
		                      HANDLER           &object,
		                      Handler_method     method,
		                      Microseconds       accuracy)
		: One_shot_timeout(timer, object, method),
		  _timer    { timer },
		  _accuracy { accuracy }
		{ }

		void schedule(Microseconds duration)
		{
			/* remove wanted deadline (may be set below) */
			_deadline_wanted.value = 0;

			/* no special treatment if timeout is unscheduled or smaller than accuracy */
			if (!scheduled() || duration.value <= _accuracy.value) {
				One_shot_timeout::schedule(duration);
				return;
			}

			using uint64_t = Genode::uint64_t;

			uint64_t const curr_time_us {
				_timer.curr_time().trunc_to_plain_us().value };

			uint64_t const deadline_us {
				duration.value <= ~(uint64_t)0 - curr_time_us ?
					curr_time_us + duration.value : ~(uint64_t)0 };

			/* new deadline is earlier */
			if (deadline_us < deadline().value) {
				/* reschedule only if old timeout is not accurate enough */
				if (deadline_us + _accuracy.value < deadline().value)
					One_shot_timeout::schedule(duration);
			}
			/* new deadline is later */
			else {
				/**
				 * remember and apply deadline in timeout handler
				 * only if old timeout is not accurate enough
				 */
				if (deadline_us - _accuracy.value > deadline().value)
					_deadline_wanted.value = deadline_us;
			}
		}
};

#endif /* _TIMEOUT_H_ */
