/*
 * \brief  Allocation and configuration helper for root and context tables
 * \author Johannes Schlatow
 * \date   2023-08-31
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVERS__PLATFORM__INTEL__MANAGED_ROOT_TABLE_H_
#define _SRC__DRIVERS__PLATFORM__INTEL__MANAGED_ROOT_TABLE_H_

/* Genode includes */
#include <base/env.h>
#include <base/attached_ram_dataspace.h>

/* local includes */
#include <intel/root_table.h>
#include <intel/context_table.h>
#include <intel/domain_allocator.h>
#include <intel/report_helper.h>
#include <hw/page_table_allocator.h>

namespace Intel {
	using namespace Genode;

	class Managed_root_table;
}


class Intel::Managed_root_table : public Registered_translation_table
{
	public:

		using Allocator = Hw::Page_table_allocator<4096>;

	private:

		Env         & _env;

		Allocator   & _table_allocator;

		addr_t        _root_table_phys  { _table_allocator.construct<Root_table>() };

		bool          _force_flush;

	public:

		addr_t phys_addr() { return _root_table_phys; }

		template <typename FN>
		void with_context_table(uint8_t bus, FN && fn)
		{
			auto no_match_fn = [&] () { };

			_table_allocator.with_table<Root_table>(_root_table_phys,
				[&] (Root_table & root_table) {

					/* allocate table if not present */
					bool new_table { false };
					if (!root_table.present(bus)) {
						root_table.address((uint8_t)bus,
						                   _table_allocator.construct<Context_table>(),
						                   _force_flush);
						new_table = true;
					}

					_table_allocator.with_table<Context_table>(root_table.address(bus),
						[&] (Context_table & ctx) {
							if (_force_flush && new_table)
								ctx.flush_all();

							fn(ctx);
						}, no_match_fn);

				}, no_match_fn);
		}

		/* add second-stage table */
		void insert_context(Pci::Bdf, addr_t, Domain_id);

		/* remove second-stage table for particular device */
		void remove_context(Pci::Bdf, addr_t);

		/* remove second-stage table for all devices */
		void remove_context(addr_t);

		/* Registered_translation_table interface */
		virtual addr_t virt_addr(addr_t pa) override
		{
			addr_t va { 0 };
			_table_allocator.with_table<Context_table>(pa,
				[&] (Context_table & table) { va = (addr_t)&table; },
				[&] ()                      { va = 0; });

			return va;
		}

		Managed_root_table(Env & env,
		                   Allocator & table_allocator,
		                   Translation_table_registry & registry,
		                   bool force_flush)
		: Registered_translation_table(registry),
		  _env(env),
		  _table_allocator(table_allocator),
		  _force_flush(force_flush)
		{ }

		~Managed_root_table() {
			_table_allocator.destruct<Root_table>(_root_table_phys); }
};

#endif /* _SRC__DRIVERS__PLATFORM__INTEL__MANAGED_ROOT_TABLE_H_ */
