/*
 * \brief  Global touch state tracker
 * \author Johannes Schlatow
 * \date   2026-01-09
 */

/*
 * Copyright (C) 2026 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _TOUCH_H_
#define _TOUCH_H_

/* local includes */
#include <types.h>
#include <pointer.h>

namespace Wm { struct Touch; }


struct Wm::Touch
{
	using Position = Pointer::Position;
	using Tracker  = Pointer::Tracker;

	class State : Noncopyable
	{
		private:

			Position _last_observed { };

			bool     _touched = false;

			Tracker &_tracker;

		public:

			State(Tracker &tracker) : _tracker(tracker) { }

			void apply_event(Input::Event const &ev)
			{
				bool pointer_report_update_needed = false;

				/* invalidate touch position on any absolute motion */
				if (ev.absolute_motion())
					_last_observed = { .valid = false, .value = { } };

				/* update touch position on first touch of a sequence */
				ev.handle_touch([&] (Input::Touch_id id, float x, float y) {
					if (id.value == 0 && !_touched) {
						_last_observed = { .valid = true, .value = { (int)x, (int)y }};
						_touched = true;
						pointer_report_update_needed = true;
					}
				});

				ev.handle_touch_release([&] (Input::Touch_id id) {
					if (id.value == 0)
						_touched = false;
				});

				if (pointer_report_update_needed)
					_tracker.update_pointer_report();
			}

			Position last_observed_pos() const { return _last_observed; }
	};
};

#endif /* _TOUCH_H_ */
