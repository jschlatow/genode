/*
 * \brief  DMA remapping hardware reporting from ACPI information in list models
 * \author Johannes Schlatow
 * \date   2023-08-14
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */


#include <base/heap.h>
#include <pci/types.h>
#include <util/list_model.h>

using namespace Genode;
using namespace Pci;


struct Drhd : List_model<Drhd>::Element
{
	using Drhd_name = String<16>;

	enum Scope { INCLUDE_PCI_ALL, EXPLICIT };

	addr_t   addr;
	size_t   size;
	unsigned segment;
	Scope    scope;
	unsigned number;

	struct Device : Registry<Device>::Element
	{
		Bdf bdf;

		Device(Registry<Device> & registry, Bdf bdf)
		: Registry<Device>::Element(registry, *this), bdf(bdf)
		{ }
	};

	Registry<Device> devices { };

	Drhd(addr_t addr, size_t size, unsigned segment, Scope scope, unsigned number)
	: addr(addr), size(size), segment(segment), scope(scope), number(number)
	{ }

	Drhd_name name() const { return Drhd_name("drhd", number); }
};


struct Drhd_policy : List_model<Drhd>::Update_policy
{
	Heap & heap;

	unsigned number { 0 };

	void destroy_element(Drhd & drhd)
	{
		drhd.devices.for_each([&] (Drhd::Device & device) {
			destroy(heap, &device); });
		destroy(heap, &drhd);
	}

	Drhd & create_element(Xml_node node)
	{
		addr_t   addr  = node.attribute_value("phys", 0UL);
		size_t   size  = node.attribute_value("size", 0UL);
		unsigned seg   = node.attribute_value("segment", 0U);
		unsigned flags = node.attribute_value("flags", 0U);

		Drhd * drhd;
		if (flags & 0x1)
			drhd = new (heap) Drhd(addr, size, seg, Drhd::Scope::INCLUDE_PCI_ALL, number++);
		else
			drhd = new (heap) Drhd(addr, size, seg, Drhd::Scope::EXPLICIT, number++);

		/* parse device scopes which define the explicitly assigned devices */
		bus_t bus = 0;
		dev_t dev = 0;
		func_t fn = 0;

		node.for_each_sub_node("scope", [&] (Xml_node node) {
			bus = node.attribute_value<uint8_t>("bus_start", 0U);
			node.with_optional_sub_node("path", [&] (Xml_node node) {
				dev = node.attribute_value<uint8_t>("dev", 0);
				fn  = node.attribute_value<uint8_t>("func", 0);
			});

			new (heap) Drhd::Device(drhd->devices, {bus, dev, fn});
		});

		return *drhd;
	}

	void update_element(Drhd &, Xml_node) {}

	static bool element_matches_xml_node(Drhd const & drhd,
	                                     Genode::Xml_node node)
	{
		addr_t addr = node.attribute_value("phys", 0UL);
		return drhd.addr == addr;
	}

	static bool node_is_element(Genode::Xml_node node) {
		return node.has_type("drhd"); }

	Drhd_policy(Heap & heap) : heap(heap) {}
};
