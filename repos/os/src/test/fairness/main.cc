/*
 * \brief  Test for scheduling fairness
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
#include <base/memory.h>
#include <base/attached_rom_dataspace.h>
#include <util/string.h>

using namespace Genode;

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


struct Main
{
	using Burner_alloc = Memory::Constrained_obj_allocator<Burner>;

	Env                          &env;
	Heap                          heap            { env.ram(), env.rm() };
	Burner_alloc                  burner_alloc    { heap };
	Attached_rom_dataspace        config          { env, "config" };

	Affinity::Space     space     { env.cpu().affinity_space() };
	Affinity::Location  location  { space.location_of_index(0) };

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

		if (config.xml().attribute_value("ep", String<8>("sleep")) == "burn") {
			for (;;);
		}
	}
};

void Component::construct(Genode::Env &env) { static Main main(env); }
