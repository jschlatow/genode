/*
 * \brief  Replacement of Timer::One_shot_timeout with lazy rescheduling
 * \author Johannes Schlatow
 * \date   2022-07-01
 *
 * NOTE: This implementation is not thread safe and should only be used in
 *       single-threaded components.
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

/* local includes */
#include <timer.h>


namespace Net {
	template <typename>
	class Lazy_one_shot_timeout;
}

template <typename HANDLER>
class Net::Lazy_one_shot_timeout : private Timer::One_shot_timeout<Lazy_one_shot_timeout<HANDLER>>
{
	private:
		using One_shot_timeout = Timer::One_shot_timeout<Lazy_one_shot_timeout<HANDLER>>;
		using Microseconds     = typename One_shot_timeout::Microseconds;
		using Duration         = typename One_shot_timeout::Duration;

		typedef void (HANDLER::*Handler_method)(Duration);

		Cached_timer         &_timer;
		HANDLER              &_object;
		Handler_method const  _method;
		Microseconds const    _accuracy;
		Microseconds          _deadline_wanted { 0 };

		using One_shot_timeout::_deadline;

		/**
		 * Evalute whether timeout needs to be scheduled immediately, lazily or
		 * can be dropped entirely. Returns true if timeout needs to be scheduled
		 * immediately.
		 *
		 * Timeouts are dropped if
		 *   old_deadline - accuracy <= new_deadline <= old_deadline + accuracy
		 *
		 * Timeouts are scheduled lazily if
		 *   new_deadline > old_deadline AND
		 *   new_deadline > old_deadline + accuracy
		 *
		 * Timeouts are scheduled immediately if
		 *   there is no active timeout OR
		 *   new_deadline < old_deadline - accuracy
		 */
		bool _needs_scheduling(Microseconds duration)
		{
			/* remove old lazy timeout (may be set below) */
			_deadline_wanted.value = 0;

			/* no special treatment if timeout is not scheduled */
			if (!scheduled()) {
				return true;
			}

			using uint64_t = Genode::uint64_t;

			uint64_t const curr_time_us {
				_timer.cached_time().trunc_to_plain_us().value };

			uint64_t const deadline_us {
				duration.value <= ~(uint64_t)0 - curr_time_us ?
					curr_time_us + duration.value : ~(uint64_t)0 };

			uint64_t const old_deadline { _deadline().value };

			/* new deadline is earlier */
			if (deadline_us < old_deadline) {
				/* reschedule only if old timeout is not accurate enough */
				if (deadline_us + _accuracy.value < old_deadline)
					return true;
			}

			/* new deadline is later */
			else {
				/**
				 * remember and apply deadline in timeout handler
				 * only if old timeout is not accurate enough
				 */
				if (deadline_us > old_deadline + _accuracy.value)
					_deadline_wanted.value = deadline_us;
			}

			return false;
		}

		/**
		 * Calculate duration from _deadline_wanted and curr_time.
		 */
		Microseconds _wanted_timeout(Duration curr_time)
		{
			Microseconds result { 0 };

			if (_deadline_wanted.value > 0) {
				Microseconds curr_time_us = curr_time.trunc_to_plain_us();
				if (curr_time_us.value + _accuracy.value < _deadline_wanted.value) {
					/* reschedule if _deadline_wanted is != 0 and in the future (minus tolerance) */
					result.value = _deadline_wanted.value - curr_time_us.value;
					_deadline_wanted.value = 0;
				}
			}

			return result;
		}

		void _handle_timeout(Duration curr_time)
		{
			_timer.curr_time(curr_time);
			Microseconds duration = _wanted_timeout(curr_time);

			if (duration.value)
				One_shot_timeout::schedule(duration);
			else
				(_object.*_method)(curr_time);
		}

	public:

		using One_shot_timeout::discard;
		using One_shot_timeout::scheduled;

		Lazy_one_shot_timeout(Cached_timer      &timer,
		                      HANDLER           &object,
		                      Handler_method     method,
		                      Microseconds       accuracy)
		: One_shot_timeout(timer, *this, &Lazy_one_shot_timeout<HANDLER>::_handle_timeout),
		  _timer    { timer },
		  _object   { object },
		  _method   { method },
		  _accuracy { accuracy }
		{ }

		void schedule(Microseconds duration)
		{
			if (_needs_scheduling(duration))
				One_shot_timeout::schedule(duration);
		}
};

#endif /* _TIMEOUT_H_ */
