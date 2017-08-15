/*
 * \brief  Policy module function declarations
 * \author Josef Soentgen
 * \date   2013-08-09
 */

/*
 * Copyright (C) 2013-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */


#include <base/stdint.h>

typedef Genode::size_t size_t;

namespace Genode {
	struct Msgbuf_base;
	struct Signal_context;
}

extern "C" size_t max_event_size ();
extern "C" size_t job_process    (char *dst, unsigned int job_id, unsigned int old_job_id);
