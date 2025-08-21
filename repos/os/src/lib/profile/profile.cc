/*
 * \brief  Instrument functions for profiling
 * \author Johannes Schlatow
 * \date   2025-08-20
 */

/*
 * Copyright (C) 2025 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/log.h>
#include <util/construct_at.h>

#include <profile/profile.h>

namespace Profile {

	Genode::uint64_t           _ticks_per_ms { 1'000'000 }; /* 1GHz TSC */
	Genode::List<Thread_info> *_threads      { nullptr };

	template <typename FN>
	void with_thread(Genode::Thread::Name const &name, FN && fn)
	{
		for (Thread_info *t = _threads->first(); t; t = t->next()) {
			if (t->name == name) {
				fn(*t);
				break;
			}
		}
	}

	template <typename FN>
	void with_function(Thread_info &th, Genode::addr_t addr, FN && fn)
	{
		bool found = false;
		for (Function_info *f = th.functions.first(); f; f = f->next()) {
			if (f->addr == addr) {
				fn(*f);
				found = true;
				break;
			}
		}

		if (!found) {
			th.obj_alloc.create(addr).with_result(
				[&] (Obj_alloc::Allocation &a) {
					a.deallocate = false;
					fn(a.obj);
					th.functions.insert(&a.obj);
				},
				[&] (Genode::Alloc_error &) {
					Genode::error(Genode::Thread::myself()->name,
					              ": Unable to allocate function info object for profiling.");
				}
			);
		}
	}

	void print_thread_info(Thread_info &th);
}


void Profile::Function_info::print()
{
	using namespace Genode;
	uint64_t const ms {  time / _ticks_per_ms };
	uint64_t const us { (time % _ticks_per_ms) / (_ticks_per_ms/1000) };
	log("  ", Hex(addr), " ", exit_count, " calls took ", ms, ".", us, " ms");
}


void Profile::print_thread_info(Thread_info &th)
{
	using namespace Genode;

	Trace::Timestamp const now = Trace::timestamp();

	if (!th.interval_ms.value || now - th.last_print < th.interval_ms.value * _ticks_per_ms)
		return;

	log("Call stack of '", th.name, "':");
	th.stack.for_each([&] (Call_stack::Entry const &e) {
		e.info->print(); });

	log("Thread '", th.name, "' profile:");
	uint64_t total_time { 0 };
	for (Function_info *f = th.functions.first(); f; f = f->next()) {
		if (f->exit_count == 0) {
			th.obj_alloc.destroy(*f);
			continue;
		}
		total_time += f->time;
		f->print();
		f->reset();
	}
	log("Total time: ", total_time / _ticks_per_ms, " ms (", (now - th.last_print) / _ticks_per_ms, " ms)");
	th.last_print = Genode::Trace::timestamp();
}


void Profile::init(Genode::uint64_t ticks_per_ms)
{
	if (ticks_per_ms > 0)
		_ticks_per_ms = ticks_per_ms;

	if (!_threads) {
		static Genode::List<Thread_info> threads { };
		_threads = &threads;
	}
}


void Profile::Thread_info::enable()
{
	if (!_threads) {
		Genode::error("Missing call to Profile::init()");
		return;
	}

	_threads->insert(this);
}


Profile::Thread_info::Thread_info(Genode::Thread::Name const &name,
                                  Genode::Allocator          &alloc,
                                  Milliseconds         const  interval)
: name(name), obj_alloc(alloc), interval_ms(interval)
{ }


extern "C" void __cyg_profile_func_enter (void *this_fn, void *)
{
	using namespace Profile;

	if (!_threads) return;

	with_thread(Genode::Thread::myself()->name, [&](Thread_info & th) {
		with_function(th, (Genode::addr_t)this_fn, [&] (Function_info & fn) {
			if (th.stack.full()) Genode::error(Genode::Thread::myself()->name,
			                                   ": Reached maximum call depth for profiling.");
			th.stack.push(&fn);
		});
	});
}


extern "C" void __cyg_profile_func_exit (void *this_fn, void *)
{
	using namespace Profile;

	if (!_threads) return;

	with_thread(Genode::Thread::myself()->name, [&](Thread_info & th) {
		th.stack.with_last([&] (Call_stack::Entry &e) {
			if (e.info->addr == (Genode::addr_t)this_fn) {
				Timestamp const time = Genode::Trace::timestamp() - e.timestamp;
				e.info->time        += time - e.callee_time;
				e.info->exit_count  += 1;

				th.stack.pop();

				/* remove 'time' from caller */
				th.stack.with_last([&] (Call_stack::Entry &caller) {
					caller.callee_time += time; });
			} else {
				Genode::error(Genode::Thread::myself()->name,
				              ": Function exit ", this_fn," does not match call stack.");
				th.stack.for_each([&] (Call_stack::Entry const &e) {
					e.info->print(); });
			}
		});
		print_thread_info(th);
	});
}
