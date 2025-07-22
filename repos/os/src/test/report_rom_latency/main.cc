/*
 * \brief  Test for report-ROM latency
 * \author Johannes Schlatow
 * \date   2025-07-21
 */

/*
 * Copyright (C) 2025 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/log.h>
#include <base/component.h>
#include <base/attached_rom_dataspace.h>
#include <os/reporter.h>
#include <timer_session/connection.h>

/* local includes */
#include <tsc_histogram.h>

namespace Test {
	struct Main;
	struct Tsc_converter;
	using namespace Genode;
}

struct Test::Tsc_converter
{
	using Type = uint32_t;

	struct Format
	{
		Type value;

		void print(Output &out) const
		{
			Type const MS = 1000, S = 1000*MS;

			using Genode::print;

			if      (value > 10*S)  print(out, value/S,  "s");
			else if (value > 10*MS) print(out, value/MS, "ms");
			else                    print(out, value,    "us");
		}
	};

	static uint64_t _calibrate(Timer::Connection &timer)
	{
		enum { SLEEP_MS = 1000, ITERATIONS = 10};
		Trace::Timestamp ticks = 0;
		for (unsigned i=ITERATIONS; i; i--) {
			Trace::Timestamp start = Trace::timestamp();
			timer.msleep(SLEEP_MS);
			ticks += (Trace::timestamp() - start)/(SLEEP_MS*1000);
		}
		return ticks / ITERATIONS;
	}

	uint64_t ticks_per_us;

	Type convert(Trace::Timestamp ts) { return (Type)(ts / ticks_per_us); }

	Tsc_converter(Timer::Connection &timer) : ticks_per_us(_calibrate(timer)) { }
};


struct Test::Main
{
	using Stats         = Genode::Histogram<80,0,40*1000,Tsc_converter>;
	using Latency_probe = Genode::Tsc_hist_probe<Stats>;

	Env &_env;

	Timer::Connection _timer { _env };
	unsigned const    _period_ms { 10 };

	Reporter               _reporter { _env, "test" };
	Attached_rom_dataspace _rom      { _env, "test"};

	unsigned                     _sample_count { 0 };
	Tsc_converter                _converter    { _timer };
	Reconstructible<Stats>       _stats        { _converter };
	Constructible<Latency_probe> _probe        { };

	void _report() {
		Reporter::Xml_generator xml(_reporter, [&] () { }); }

	void _handle_rom_update()
	{
		_probe.destruct();
		_timer.trigger_once(1000*_period_ms);
	}

	Signal_handler<Main> _rom_update_handler {
		_env.ep(), *this, &Main::_handle_rom_update };

	void _handle_timer()
	{
		_sample_count++;
		if (_sample_count % 1000 == 0 && _stats.constructed()) {
			_stats->output_stats<Log::Log_fn>();
			_stats->output_visual_stats<Log::Log_fn>();
			log("\nHistogram:");
			_stats->output_histogram<Log::Log_fn>();
			_stats.construct(_converter);
		}

		_probe.construct(*_stats);
		_report();
	}

	Signal_handler<Main> _timer_handler {
		_env.ep(), *this, &Main::_handle_timer };

	Main(Env &env) : _env(env)
	{
		_timer.sigh(_timer_handler);
		_reporter.enabled(true);
		_rom.sigh(_rom_update_handler);

		_timer.trigger_once(1000*_period_ms);
	}
};


void Component::construct(Genode::Env &env) { static Test::Main main(env); }
