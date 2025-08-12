/*
 * \brief  Testing RDTSCP on multiple cores
 * \author Johannes Schlatow
 * \date   2025-08-07
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
#include <util/reconstructible.h>
#include <timer_session/connection.h>

using namespace Genode;

struct Test : Thread
{
	List_element<Test>     _list_element { this };
	Env                   &_env;
	Timer::Connection      _timer { _env };
	uint64_t               _period_offset;
	uint64_t volatile      _last_tsc    { 0 };
	uint32_t volatile      _aux { 0 };

	Test(Env &env, Location const &location, uint64_t offset)
	:
		Thread(env, Name("rdtscp_", location.xpos(), "x", location.ypos()),
		       4 * 4096, location, Weight(), env.cpu()),
		_env(env), _period_offset(offset)
	{ }

	void entry() override
	{
		while (true) {
			uint32_t lo, hi;
			asm volatile ("rdtscp" : "=a" (lo), "=d" (hi), "=c" (_aux) :: "memory");
			_last_tsc = (uint64_t)hi << 32 | lo;
			_timer.msleep(30000-_period_offset);
			
			/*
			 * XXX without this, we get a relatively large TSC difference when the
			 * oberver runs on the same core as the timer component
			 */
			for (size_t i=0; i < 20000; i++) {
				asm volatile("nop");
			}
		}
	}
};


using Thread_list = List<List_element<Test> >;

struct Observer : Thread
{
	Env              &_env;
	Thread_list      &_threads;
	uint64_t          _sequence_id { 0 };

	Observer(Env &env, Location const &location, Thread_list &threads)
	:
		Thread(env, Name("observer_", location.xpos(), "x", location.ypos()),
		       4 * 4096, location, Weight(), env.cpu()),
		_env(env), _threads(threads)
	{ }

	void entry() override
	{
		while(true) {
			_sequence_id++;

			for (auto t = _threads.first(); t; t = t->next()) {
				auto thread = t->object();
				if (!thread)
					continue;

				uint64_t last_thread_tsc = thread->_last_tsc;
				uint64_t thread_tsc = last_thread_tsc;
				uint64_t ref_tsc;
				while (thread_tsc == last_thread_tsc) {
					thread_tsc = thread->_last_tsc;
					ref_tsc    = Trace::timestamp();
				}

				log(_sequence_id, ":", thread->_aux, " ", Hex(thread_tsc), "-", Hex(ref_tsc), " = ", (int64_t)thread_tsc - (int64_t)ref_tsc);
			}
		}
	}
};


struct Main
{
	Env              &_env;
	Heap              _heap   { _env.ram(), _env.rm() };
	Timer::Connection _timer  { _env };

	Thread_list             _threads { };
	Constructible<Observer> _observer { };

	Main(Env &env) : _env(env)
	{
		Affinity::Space space = env.cpu().affinity_space();

		for (unsigned i = 0; i < space.total(); i++) {
			Affinity::Location location = env.cpu().affinity_space().location_of_index(i);

			if (i == 2) {
				_observer.construct(env, location, _threads);
			} else {
				Test *t = new (_heap) Test(env, location, i*1000);
				t->start();

				_threads.insert(&t->_list_element);
			}
		}

		_observer->start();
	}
};


void Component::construct(Env &env)
{
	static Main main(env);
}
