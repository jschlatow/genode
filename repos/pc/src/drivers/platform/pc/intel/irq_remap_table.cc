/*
 * \brief  Intel IOMMU Interrupt Remapping Table implementation
 * \author Johannes Schlatow
 * \date   2023-11-09
 *
 * The interrupt remapping table is a page-aligned table structure of up to 64K
 * 128bit entries (see section 9.9 [1]). Each entries maps a virtual interrupt
 * index to a destination ID and vector.
 *
 * [1] "Intel® Virtualization Technology for Directed I/O"
 *     Revision 4.1, March 2023
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* local includes */
#include <intel/irq_remap_table.h>

Intel::Irq_remap::Hi::access_t Intel::Irq_remap::hi_val(Pci::Bdf const & bdf)
{
	return Hi::Svt::bits(Hi::Svt::SOURCE_ID) |
	       Hi::Sq::bits(Hi::Sq::ALL_BITS) |
	       Hi::Source_id::bits(Pci::Bdf::rid(bdf));
}


Intel::Irq_remap::Lo::access_t Intel::Irq_remap::lo_val(Irq_session::Info const & info)
{
	Irq_address::access_t address = info.address;
	Irq_data::access_t    data    = info.value;

	return
		Lo::Present::bits(1) |
		Lo::Destination_mode::bits(Irq_address::Destination_mode::get(address)) |
		Lo::Redirection_hint::bits(Irq_address::Redirection_hint::get(address)) |
		Lo::Trigger_mode::bits(Irq_data::Trigger_mode::get(data)) |
		Lo::Delivery_mode::bits(Irq_data::Delivery_mode::get(data)) |
		Lo::Vector::bits(Irq_data::Vector::get(data));
}
