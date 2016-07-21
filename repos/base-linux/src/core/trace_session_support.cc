/*
 * \brief   Platform specific parts of TRACE session
 * \author  Johannes Schlatow
 * \date    2016-07-21
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <dataspace/capability.h>

/* core includes */
#include <trace/session_component.h>

using namespace Genode;

Dataspace_capability Genode::Trace::Session_component::core_buffer()
{
	return Dataspace_capability();
}
