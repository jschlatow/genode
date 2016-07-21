/*
 * \brief  Create CPU load for core tracing
 * \author Johannes Schlatow
 * \date   2016-07-21
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/component.h>


/************
 ** Server **
 ************/

namespace Component
{
	Genode::size_t stack_size() { return 2 * 1024 * sizeof(long); }

	void construct(Genode::Env &env) {
		while (1) {	}
	}
}
