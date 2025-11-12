/*
 * \brief  Sequence number generator
 * \author Johannes Schlatow
 * \date   2025-11-12
 */

/*
 * Copyright (C) 2025 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SEQ_NUMBER_GENERATOR_H_
#define _SEQ_NUMBER_GENERATOR_H_

/* Genode includes */
#include <input/event.h>
#include <input/component.h>

namespace Wm {

	struct Seq_number_generator;
}

struct Wm::Seq_number_generator
{
	bool              _clicked    { false };
	bool              _submitted  { false };
	Input::Seq_number _seq_number { };

	static bool click(Input::Event const &event)
	{
		bool result = false;

		if (event.key_press(Input::BTN_LEFT))
			result = true;

		event.handle_touch([&] (Input::Touch_id id, float, float) {
			if (id.value == 0)
				result = true; });

		return result;
	}

	static bool clack(Input::Event const &event)
	{
		bool result = false;

		if (event.key_release(Input::BTN_LEFT))
			result = true;

		event.handle_touch_release([&] (Input::Touch_id id) {
			if (id.value == 0)
				result = true; });

		return result;
	}

	void new_seq_number()
	{
		_seq_number.value++;
		_submitted = false;
	}

	void apply_event(Input::Event const &);
	void submit(Input::Session_component &);
};


void Wm::Seq_number_generator::apply_event(Input::Event const &event)
{
	bool const orig_clicked = _clicked;

	if (click(event)) _clicked = true;
	if (clack(event)) _clicked = false;

	if (!orig_clicked && _clicked)
		new_seq_number();
}


void Wm::Seq_number_generator::submit(Input::Session_component &input)
{
	if (_submitted)
		return;

	input.submit(_seq_number);
	_submitted = true;
}

#endif /* _SEQ_NUMBER_GENERATOR_H_ */
