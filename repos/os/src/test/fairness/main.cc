/*
 * \brief  Test for scheduling fairness
 * \author Johannes Schlatow
 * \date   2025-06-02
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
#include <base/memory.h>
#include <base/attached_rom_dataspace.h>
#include <util/string.h>

using namespace Genode;

static Mutex &mutex()
{
	static Mutex inst;
	return inst;
}

/**
 * A mutex thread
 */
class Mutex_thread : Thread
{
	private:

		bool     volatile  _stop       { false };

		void entry() override
		{
			while (!_stop) {
				mutex().acquire();
				mutex().release();
			}
		}

	public:
	
		Mutex_thread(Env                       &env,
		             Affinity::Location  const &location,
		             String<32>          const &name)
		:
			Thread(env, name, 8*1024, location, Weight(), env.cpu())
		{
			Thread::start();
		}

		~Mutex_thread()
		{
			if (!_stop && Thread::myself() != this) {
				_stop = true;
				join();
			}
		}
 };

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
		       Affinity::Location  const &location,
		       String<32>          const &name)
		:
			Thread(env, name, 8*1024, location, Weight(), env.cpu())
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
 * A signal sender thread
 */
class Sender : Thread
{
	private:

		Signal_context_capability cap;

		void entry() override
		{
			while (true)
				Signal_transmitter(cap).submit();
		}

	public:

		Sender(Env                       &env,
		       Affinity::Location  const &location,
		       Signal_context_capability  cap)
		:
			Thread(env, "sender_thread", 8*1024, location, Weight(), env.cpu()), cap(cap)
		{
			Thread::start();
		}
};


struct Main
{
	using Burner_alloc       = Memory::Constrained_obj_allocator<Burner>;
	using Mutex_thread_alloc = Memory::Constrained_obj_allocator<Mutex_thread>;

	Env                          &env;
	Heap                          heap               { env.ram(), env.rm() };
	Burner_alloc                  burner_alloc       { heap };
	Mutex_thread_alloc            mutex_thread_alloc { heap };
	Attached_rom_dataspace        config             { env, "config" };

	Io_signal_handler<Main>       signal_handler     { env.ep(), *this, &Main::handle };
	Constructible<Sender>         sender             { };

	Affinity::Space     space     { env.cpu().affinity_space() };
	Affinity::Location  location  { space.location_of_index(0) };

	void handle()
	{ }

	Main(Env &env) : env(env)
	{
		log("--- Fairness test ---");

		/* read threads from config */
		config.xml().for_each_sub_node("thread", [&] (Xml_node const &node) {
			String<32> name = node.attribute_value("name", String<32>(""));

			burner_alloc.create(env, location, name).with_result(
				[&] (Burner_alloc::Allocation &a) {
					a.deallocate = false;
					log("Created burner thread\"", name, "\"");
				},
				[&] (Alloc_error) {
					error("Failed to create burner thread \"", name, "\"");
				}
			);
		});

		config.xml().for_each_sub_node("mutex_thread", [&] (Xml_node const &node) {
			String<32> name = node.attribute_value("name", String<32>(""));

			mutex_thread_alloc.create(env, location, name).with_result(
				[&] (Mutex_thread_alloc::Allocation &a) {
					a.deallocate = false;
					log("Created mutex thread\"", name, "\"");
				},
				[&] (Alloc_error) {
					error("Failed to create mutex thread \"", name, "\"");
				}
			);
		});

		String<8> ep_attr = config.xml().attribute_value("ep", String<8>("sleep"));
		if (ep_attr == "burn") {
			for (;;);
		} else if (ep_attr == "receive") {
			sender.construct(env, location, signal_handler);

			for (;;) {
				env.ep().wait_and_dispatch_one_io_signal();
			}
		}
	}
};

void Component::construct(Genode::Env &env) { static Main main(env); }
