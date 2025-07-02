/*
 * \brief  Test for memcpy performance under background load
 * \author Johannes Schlatow
 * \date   2025-07-02
 */

/*
 * Copyright (C) 2025 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/log.h>
#include <base/component.h>
#include <timer_session/connection.h>
#include <base/attached_ram_dataspace.h>

struct Main
{
	Genode::Env &env;

	Timer::Connection timer { env };

	Genode::Attached_ram_dataspace cached_ds1 { env.ram(), env.rm(), 4096, Genode::CACHED };
	Genode::Attached_ram_dataspace cached_ds2 { env.ram(), env.rm(), 4096, Genode::CACHED };

	Genode::Signal_handler<Main> timer_handler { env.ep(), *this, &Main::handle_timer };

	void handle_timer();
	
	Main(Genode::Env &env);
};


void Main::handle_timer()
{
	using namespace Genode;
	for (size_t i=0; i < 2; i++) {
		GENODE_LOG_TSC(2);
		memcpy(cached_ds1.local_addr<void>(), cached_ds2.local_addr<void>(), 4096);
	}
}


Main::Main(Genode::Env &env) : env(env)
{
	using namespace Genode;

	timer.sigh(timer_handler);
	timer.trigger_periodic((uint64_t)1000*1000);
}



void Component::construct(Genode::Env &env) { static Main inst(env); }

