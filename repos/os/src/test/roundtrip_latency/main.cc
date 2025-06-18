/*
 * \brief  Test for roundtrip signalling latency
 * \author Johannes Schlatow
 * \date   2025-04-01
 */

/*
 * Copyright (C) 2025 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/component.h>
#include <base/heap.h>
#include <base/thread.h>
#include <base/registry.h>
#include <base/duration.h>
#include <base/attached_rom_dataspace.h>
#include <timer_session/connection.h>

/* local includes */
#include <tsc_histogram.h>

using namespace Genode;

struct Converter
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

	Type convert(Trace::Timestamp ts) { return (Type)ts; }
};


struct Tsc_converter : Converter
{
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


using Stats      = Histogram<80,0,40*1000,Tsc_converter>;
using Meta_stats = Histogram<80,0,40*1000,Converter>;
using Latency_probe = Tsc_hist_probe<Stats>;


/**
 * A burner thread
 */
class Burner : Thread
{
	private:

		bool     volatile  _stop       { false };

		void entry() override
		{
			while (!_stop) { }
		}

	public:

		Burner(Env                       &env,
		       Affinity::Location  const &location)
		:
			Thread(env, "burner", 8*1024, location, Weight(), env.cpu())
		{
			Thread::start();
		}

		~Burner()
		{
			if (!_stop && Thread::myself() != this) {
				_stop = true;
				join();
			}
		}
};


/**
 * A thread that submits a signal context in a periodic fashion
 */
class Sender : Thread
{
	private:

		Timer::Connection  _timer;
		Signal_transmitter _transmitter;
		Signal_receiver   &_receiver;
		Milliseconds const _interval;
		Stats             &_stats;
		bool     volatile  _stop       { false };

		void entry() override
		{
			while (!_stop) {
				{
					Latency_probe probe(_stats);

					_transmitter.submit();
					_receiver.wait_for_signal();
				}

				if (_interval.value) {
					_timer.msleep(_interval.value); }
			}
		}

	public:

		Sender(Env                       &env,
		       Affinity::Location  const &location,
		       Signal_context_capability  context,
		       Signal_receiver           &receiver,
		       Milliseconds               interval,
		       Stats                     &stats)
		:
			Thread(env, "sender", 8*1024, location, Weight(), env.cpu()),
			_timer(env),
			_transmitter(context),
			_receiver(receiver),
			_interval(interval),
			_stats(stats)
		{
			Thread::start();
		}

		~Sender()
		{
			if (!_stop && Thread::myself() != this) {
				_stop = true;
				join();
			}
		}
};


/**
 * A thread that receives signals and sends them back
 */
class Handler : Thread
{
	private:

		Signal_transmitter _transmitter;
		Signal_receiver   &_receiver;
		bool               _stop           { false };

		uint8_t            _buffer1[4096];
		uint8_t            _buffer2[4096];

		void entry() override
		{
			while (!_stop) {
				Signal signal = _receiver.wait_for_signal();
				if (signal.num() > 1)
					error("Got signalled more than once");

				/* let's consume some CPU time to make preemptions measurable
				 * remark: number of iterations was carefully adjusted to result in
				 *         ~170us on a Thinkpad x230, which turned out to be just
				 *         manageable by NOVA
				 */
				for (unsigned i=0; i < 380; i++)
					memcpy(_buffer1, _buffer2, sizeof(_buffer1));

				/* signal back */
				if (!_stop)
					_transmitter.submit();
			}
		}

	public:

		Handler(Env                       &env,
		        Affinity::Location  const &location,
		        Signal_context_capability  context,
		        Signal_receiver           &receiver)
		:
			Thread(env, "handler", 8*1024, location, Weight(), env.cpu()),
			_transmitter(context),
			_receiver(receiver)
		{
			Thread::start();
		}

		~Handler()
		{
			/* signal thread to exit loop */
			Signal_context            context;
			Signal_context_capability context_cap { _receiver.manage(context) };

			_stop = true;
			Signal_transmitter(context_cap).submit();
			Thread::join();
			_receiver.dissolve(context);
		}
};


struct Test
{
	int                id;
	Stats              stats;
	Signal_context     context         { };
	Signal_context     context_return  { };
	Signal_receiver    receiver        { };
	Signal_receiver    receiver_return { };
	bool const         verbose   { false };

	Constructible<Handler> handler { };
	Constructible<Sender>  sender { };

	void stop()
	{
		sender.destruct();
		handler.destruct();
	}

	Test(Env &env, Affinity::Location const &location, int id,
	     Milliseconds interval, Tsc_converter &converter)
	: id(id), stats(converter)
	{
		if (verbose)
			log("Starting sender/receiver ", id, "; sleep_ms=", interval.value);

		handler.construct(env, location,
		                  receiver_return.manage(context_return), receiver);
		sender.construct(env, location,
		                 receiver.manage(context), receiver_return,
		                 interval, stats);
	}

	virtual ~Test() { stop(); }

};

struct Main
{
	Env                   &env;
	Heap                   heap      { env.ram(), env.rm() };
	Timer::Connection      timer     { env };
	Tsc_converter          converter { timer };

	Affinity::Space        space     { env.cpu().affinity_space() };
	Affinity::Location     location  { space.location_of_index(space.total()-1) };

	Attached_rom_dataspace config { env, "config" };

	unsigned               next_id   { 0 };

	Constructible<Test>        single_test { };
	Constructible<Burner>      burner { };
	Registry<Registered<Test>> registry { };

	void _bench_mode()
	{
		log("--- Roundtrip-latency test ---");

		test(1, Milliseconds { 0 });
		print_stats();

		log("--- running 2 tests in parallel ---");

		test(2, Milliseconds { 0 });
		print_diff_stats();

		log("--- adding same-priority burner ---");

		burner.construct(env, location);
		test(2, Milliseconds { 0 });
		burner.destruct();
		print_diff_stats();

		unsigned num = min(max_tests(), 64U);
		log("--- running ", num, " tests in parallel ---");

		test(num, Milliseconds { 10 });
		print_combined_stats();

		log("--- Roundtrip-latency test finished ---");
		env.parent().exit(0);
	}

	void _continuous_mode()
	{
		log("--- Measuring roundtrip latency (continuous mode) ---");

		for (;;) {
			test(1, Milliseconds{ 10 });
			print_diff_stats();
		}
	}

	unsigned max_tests()
	{
		
		addr_t const max_threads = Thread::stack_area_virtual_size() /
		                           Thread::stack_virtual_size();

		return (unsigned)max_threads/2;
	}

	Main(Env &env) : env(env)
	{
		if (config.xml().attribute_value("continuous", false))
			_continuous_mode();
		else
			_bench_mode();
	}

	void test(unsigned num, Milliseconds interval)
	{
		for (unsigned i=num; i; i--)
			new (heap) Registered<Test>(registry, env, location,
			                            next_id++, interval, converter);

		/* collect data for 3s */
		timer.msleep(3000);
	}

	void print_stats()
	{
		registry.for_each([&] (Test &test) {
			test.stop();

			log("\nSender ", test.id, " stats:");
			test.stats.output_stats<Log::Log_fn>();
			test.stats.output_visual_stats<Log::Log_fn>();
			log("\nHistogram:");
			test.stats.output_histogram<Log::Log_fn>();
			destroy(heap, &test);
		});
	}

	void print_diff_stats()
	{
		registry.for_each([&] (Test &test) {
			test.stop();

			test.stats.output_visual_stats<Log::Log_fn>();

			destroy(heap, &test);
		});
	}

	void print_combined_stats()
	{
		/* stop and combine stats */
		Converter  meta_converter { };
		Stats      stats { converter };
		Meta_stats median_stats { meta_converter };
		Meta_stats p90_stats    { meta_converter };
		registry.for_each([&] (Test &test) {
			test.stop();
			stats += test.stats;
			median_stats.add(test.stats.percentile(50));
			p90_stats.   add(test.stats.percentile(90));
			destroy(heap, &test);
		});

		/* print histogram over all samples */
		stats.output_histogram<Log::Log_fn>();

		/* print stats of medians and 90th percentile -> variation should be small */
		log("\nStats of medians:");
		median_stats.output_visual_stats<Log::Log_fn>();
		log("\nStats of 90th percentiles:");
		p90_stats.output_visual_stats<Log::Log_fn>();
	}
};

void Component::construct(Genode::Env &env) { static Main main(env); }
