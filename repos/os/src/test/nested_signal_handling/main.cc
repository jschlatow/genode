/*
 * \brief  Test for nested signal handling
 * \author Johannes Schlatow
 * \date   2025-11-20
 *
 * This test exercises the signalling and update protocol of the dynamic ROM
 * session. It periodically signals the ROM client, which is then supposed
 * to request the current dataspace and thereby update the ROM content.
 * Whenever the time between the signal and the client-initiated ROM update
 * is too long, we know that the client missed a signal.
 */

/*
 * Copyright (C) 2025 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/log.h>
#include <base/heap.h>
#include <base/session_object.h>
#include <root/component.h>
#include <libc/component.h>
#include <timer_session/connection.h>
#include <os/dynamic_rom_session.h>
#include <trace/timestamp.h>

/* libc includes */
#include <sys/stat.h>

namespace Test {
	class Main;
	class Consumer;
	using namespace Genode;
}

struct Test::Consumer
{
	Env                      &_env;
	Timer::Connection         _timer  { _env };
	Rom_connection            _rom    { _env, "test" };
	Signal_handler<Consumer>  _rom_handler { _env.ep(), *this, &Consumer::_handle_rom };
	unsigned                  _cnt { 0 };
	Trace::Timestamp          _tsc_per_ms { 0 };
	Trace::Timestamp          _start_ts { 0 };

	void _handle_rom()
	{
		Trace::Timestamp const ts = Trace::timestamp();
		_rom.update();

		/* spend some time to make sure that producer is scheduled */
		while (((Trace::timestamp() - ts) / _tsc_per_ms) < 11)
			asm volatile ("nop");

		/* do some libc file operation */
		struct stat s;
		s.st_size = 0;
		Libc::with_libc([&] () { stat("testfile", &s); });

		if (((ts - _start_ts) / _tsc_per_ms) > 10'000)
			_env.parent().exit(0);
	}

	Consumer(Env & env)
	: _env(env),
	  _start_ts(Trace::timestamp())
	{
		/* calibrate tsc */
		enum { SLEEP_MS = 1000 };
		_timer.msleep(SLEEP_MS);
		_tsc_per_ms = (Trace::timestamp() - _start_ts) / SLEEP_MS;

		_rom.sigh(_rom_handler);
		_handle_rom();
	}
};


struct Test::Main : Dynamic_rom_session::Producer
{
	Env &_env;
	Heap _heap { _env.ram(), _env.rm() };

	unsigned _cnt { 0 };

	Timer::Connection _timer { _env };

	void _handle_timeout();

	void _handle_trigger()
	{
		if (_rom_root._session.constructed())
			_rom_root._session->trigger_update();
	}

	Signal_handler<Main> _timeout_handler {
		_env.ep(), *this, &Main::_handle_timeout };

	Signal_handler<Main> _trigger_handler {
		_env.ep(), *this, &Main::_handle_trigger };

	/* Dynamic_rom_session::Producer API */
	void generate(Generator &) override;

	struct Rom_root : Root_component<Dynamic_rom_session, Genode::Single_client>
	{
		Env  &_env;
		Main &_main;

		Constructible<Dynamic_rom_session> _session { };

		Create_result _create_session(const char *) override
		{
			_session.construct(_env.ep().rpc_ep(), _env.ram(), _env.rm(), _main);
			return *_session;
		}

		void _destroy_session(Dynamic_rom_session &) override {
			_session.destruct(); }
		
		Rom_root(Env &env, Allocator &md_alloc, Main &main)
		:
			Root_component<Dynamic_rom_session, Genode::Single_client>(&env.ep().rpc_ep(), &md_alloc),
			_env(env), _main(main)
		{ }

	} _rom_root { _env, _heap, *this };

	Main(Env &env)
	: Dynamic_rom_session::Producer("test"),
	  _env(env)
	{
		_timer.sigh(_timeout_handler);

		_env.parent().announce(_env.ep().manage(_rom_root));
	}
};


void Test::Main::_handle_timeout()
{
	error("Consumer missed a signal ", _cnt);
	_env.parent().exit(1);
}


void Test::Main::generate(Generator & g)
{
	/* 1s timeout */
	_timer.trigger_once(1000*1000);

	g.attribute("value", _cnt++);

	/* signal myself to trigger update */
	Signal_transmitter(_trigger_handler).submit();
}


/***************
 ** Component **
 ***************/

void Libc::Component::construct(Libc::Env &env)
{
	using namespace Genode;

	/*
	 * Read value 'consumer' attribute to decide whether to perform
	 * the consumer or producer role.
	 */
	static Attached_rom_dataspace config(env, "config");
	bool const is_consumer = config.node().attribute_value("consumer", false);

	Libc::with_libc([&] {
		if (is_consumer) {
			log("--- test-dynamic_rom consumer started ---");
			static Test::Consumer consumer(env);
		} else {
			log("--- test-dynamic_rom producer started ---");
			static Test::Main producer(env);
		}
	});

}
