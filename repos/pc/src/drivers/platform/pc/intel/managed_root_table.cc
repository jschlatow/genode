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

/* local includes */
#include <intel/managed_root_table.h>

void Intel::Managed_root_table::insert_context(Pci::Bdf  bdf,
                                               addr_t    phys_addr,
                                               Domain_id domain)
{
	with_context_table(bdf.bus, [&] (Context_table & ctx) {
		Pci::rid_t      rid = Pci::Bdf::rid(bdf);

		if (ctx.present(rid))
			error("Translation table already set for ", bdf);

		ctx.insert(rid, phys_addr, domain.value, _force_flush);
	});
}

void Intel::Managed_root_table::remove_context(Pci::Bdf  bdf,
                                               addr_t    phys_addr)
{
	with_context_table(bdf.bus, [&] (Context_table & ctx) {
		Pci::rid_t      rid = Pci::Bdf::rid(bdf);

		if (ctx.stage2_pointer(rid) != phys_addr)
			error("Trying to remove foreign translation table for ", bdf);

		ctx.remove(rid, _force_flush);
	});
}


void Intel::Managed_root_table::remove_context(addr_t phys_addr)
{
	for (size_t bus=0; bus < 256; bus++) {
		with_context_table((uint8_t)bus, [&] (Context_table & ctx) {
			for (Pci::rid_t id=0; id < 256; id++) {
				if (!ctx.present(id))
					continue;

				if (ctx.stage2_pointer(id) != phys_addr)
					continue;

				Pci::Bdf bdf = { (Pci::bus_t) bus,
				                 (Pci::dev_t) Pci::Bdf::Routing_id::Device::get(id),
				                 (Pci::func_t)Pci::Bdf::Routing_id::Function::get(id) };
				remove_context(bdf, phys_addr);
			}
		});
	}
}
