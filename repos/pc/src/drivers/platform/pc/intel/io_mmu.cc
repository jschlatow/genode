/*
 * \brief  Intel IOMMU implementation
 * \author Johannes Schlatow
 * \date   2023-08-15
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* local includes */
#include <intel/io_mmu.h>

using namespace Driver;

static void attribute_hex(Genode::Xml_generator & xml, char const * name,
                          unsigned long long value)
{
	xml.attribute(name, Genode::String<32>(Genode::Hex(value)));
}


void Intel::Io_mmu::Domain::enable_pci_device(Pci::Bdf const bdf)
{
	_intel_iommu.root_table().insert_context(bdf, _translation_table_phys,
	                                         _domain_id);

	/* invalidate translation caches only if failed requests are cached */
	if (_intel_iommu.caching_mode())
		_intel_iommu.invalidate_all(_domain_id, Pci::Bdf::rid(bdf));
}


void Intel::Io_mmu::Domain::disable_pci_device(Pci::Bdf const bdf)
{
	_intel_iommu.root_table().remove_context(bdf, _translation_table_phys);

	_intel_iommu.invalidate_all(_domain_id);
}


void Intel::Io_mmu::Domain::add_range(Range const & range,
                                      addr_t const paddr,
                                      Dataspace_capability const)
{
	addr_t const             vaddr   { range.start };
	size_t const             size    { range.size };

	Page_flags flags { RW, NO_EXEC, USER, NO_GLOBAL,
	                   RAM, Genode::CACHED };

	_translation_table.insert_translation(vaddr, paddr, size, flags,
	                                      _table_allocator,
	                                      !_intel_iommu.coherent_page_walk());

	/* only invalidate iotlb if failed requests are cached */
	if (_intel_iommu.caching_mode())
		_intel_iommu.invalidate_iotlb(_domain_id, vaddr, size);
}


void Intel::Io_mmu::Domain::remove_range(Range const & range)
{
	_translation_table.remove_translation(range.start, range.size,
	                                      _table_allocator,
	                                      !_intel_iommu.coherent_page_walk());

	_intel_iommu.invalidate_iotlb(_domain_id, range.start, range.size);
}


/**
 * Clear IOTLB.
 *
 * By default, we perform a global invalidation. When provided with a valid
 * Domain_id, a domain-specific invalidation is conducted. If provided with
 * a DMA address and size, a page-selective invalidation is performed.
 *
 * See Table 25 for required invalidation scopes.
 */
void Intel::Io_mmu::invalidate_iotlb(Domain_id domain_id, addr_t, size_t)
{

	unsigned requested_scope = Context_command::Cirg::GLOBAL;
	if (domain_id.valid())
		requested_scope = Context_command::Cirg::DOMAIN;

	/* wait for ongoing invalidation request to be completed */
	while (Iotlb::Invalidate::get(read_iotlb_reg()));

	/* invalidate IOTLB */
	write_iotlb_reg(Iotlb::Invalidate::bits(1) |
	                Iotlb::Iirg::bits(requested_scope) |
	                Iotlb::Dr::bits(1) | Iotlb::Dw::bits(1) |
	                Iotlb::Did::bits(domain_id.value));

	/* wait for completion */
	while (Iotlb::Invalidate::get(read_iotlb_reg()));

	/* check for errors */
	unsigned actual_scope = Iotlb::Iaig::get(read_iotlb_reg());
	if (!actual_scope)
		error("IOTLB invalidation failed (scope=", requested_scope, ")");
	else if (_verbose && actual_scope < requested_scope)
		warning("Performed IOTLB invalidation with different granularity ",
		        "(requested=", requested_scope, ", actual=", actual_scope, ")");

	/* XXX implement page-selective-within-domain IOTLB invalidation */
}

/**
 * Clear context cache and IOTLB.
 *
 * By default, we perform a global invalidation. When provided with a valid
 * Domain_id, a domain-specific invalidation is conducted. When a rid is 
 * provided, a device-specific invalidation is done.
 *
 * See Table 25 for required invalidation scopes.
 */
void Intel::Io_mmu::invalidate_all(Domain_id domain_id, Pci::rid_t rid)
{
	/**
	 * We are using the register-based invalidation interface for the
	 * moment. This is only supported in legacy mode and for major
	 * architecture version 5 and lower (cf. 6.5).
	 */

	if (read<Version::Major>() > 5) {
		error("Unable to invalidate caches: Register-based invalidation only ",
		      "supported in architecture versions 5 and lower");
		return;
	}

	/* make sure that there is no context invalidation ongoing */
	while (read<Context_command::Invalidate>());

	unsigned requested_scope = Context_command::Cirg::GLOBAL;
	if (domain_id.valid())
		requested_scope = Context_command::Cirg::DOMAIN;

	if (rid != 0)
		requested_scope = Context_command::Cirg::DEVICE;

	/* clear context cache */
	write<Context_command>(Context_command::Invalidate::bits(1) |
	                       Context_command::Cirg::bits(requested_scope) |
	                       Context_command::Sid::bits(rid) |
	                       Context_command::Did::bits(domain_id.value));


	/* wait for completion */
	while (read<Context_command::Invalidate>());

	/* check for errors */
	unsigned actual_scope = read<Context_command::Caig>();
	if (!actual_scope)
		error("Context-cache invalidation failed (scope=", requested_scope, ")");
	else if (_verbose && actual_scope < requested_scope)
		warning("Performed context-cache invalidation with different granularity ",
		        "(requested=", requested_scope, ", actual=", actual_scope, ")");

	/* XXX clear PASID cache if we ever switch from legacy mode translation */

	invalidate_iotlb(domain_id, 0, 0);
}


void Intel::Io_mmu::_handle_faults()
{
	if (_fault_irq.constructed())
		_fault_irq->ack_irq();

	if (read<Fault_status::Pending>()) {
		if (read<Fault_status::Overflow>())
			error("Fault recording overflow");

		if (read<Fault_status::Iqe>())
			error("Invalidation queue error");

		/* acknowledge all faults */
		write<Fault_status>(0x7d);

		error("Faults records for ", name());
		unsigned num_registers = read<Capability::Nfr>() + 1;
		for (unsigned i = read<Fault_status::Fri>(); ; i = (i + 1) % num_registers) {
			Fault_record_hi::access_t hi = read_fault_record<Fault_record_hi>(i);

			if (!Fault_record_hi::Fault::get(hi))
				break;

			Fault_record_hi::access_t lo = read_fault_record<Fault_record_lo>(i);

			error("Fault: hi=", Hex(hi),
			      ", reason=", Hex(Fault_record_hi::Reason::get(hi)),
			      ", type=",   Hex(Fault_record_hi::Type::get(hi)),
			      ", AT=",     Hex(Fault_record_hi::At::get(hi)),
			      ", EXE=",    Hex(Fault_record_hi::Exe::get(hi)),
			      ", PRIV=",   Hex(Fault_record_hi::Priv::get(hi)),
			      ", PP=",     Hex(Fault_record_hi::Pp::get(hi)),
			      ", Source=", Hex(Fault_record_hi::Source::get(hi)),
			      ", info=",   Hex(Fault_record_lo::Info::get(lo)));


			clear_fault_record(i);
		}
	}
}


void Intel::Io_mmu::generate(Xml_generator & xml)
{
	xml.node("intel_iommu", [&] () {
		xml.attribute("name", name());

		/* dump registers */
		xml.attribute("version", String<16>(read<Version::Major>(), ".",
		                                    read<Version::Minor>()));

		xml.node("register", [&] () {
			xml.attribute("name", "Capability");
			attribute_hex(xml, "value", read<Capability>());
			xml.attribute("esrtps", (bool)read<Capability::Esrtps>());
			xml.attribute("rwbf",   (bool)read<Capability::Rwbf>());
			xml.attribute("nfr",     read<Capability::Nfr>());
			xml.attribute("domains", read<Capability::Domains>());
			xml.attribute("caching", (bool)read<Capability::Caching_mode>());
		});

		xml.node("register", [&] () {
			xml.attribute("name", "Extended Capability");
			attribute_hex(xml, "value", read<Extended_capability>());
			xml.attribute("page_walk_coherency", (bool)read<Extended_capability::Page_walk_coherency>());
		});

		xml.node("register", [&] () {
			xml.attribute("name", "Global Status");
			attribute_hex(xml, "value", read<Global_status>());
			xml.attribute("qies",    (bool)read<Global_status::Qies>());
			xml.attribute("rtps",    (bool)read<Global_status::Rtps>());
			xml.attribute("irtps",   (bool)read<Global_status::Irtps>());
			xml.attribute("enabled", (bool)read<Global_status::Enabled>());
		});

		xml.node("register", [&] () {
			xml.attribute("name", "Fault Status");
			attribute_hex(xml, "value", read<Fault_status>());
			attribute_hex(xml, "fri",   read<Fault_status::Fri>());
			xml.attribute("iqe", (bool)read<Fault_status::Iqe>());
			xml.attribute("ppf", (bool)read<Fault_status::Pending>());
			xml.attribute("pfo", (bool)read<Fault_status::Overflow>());
		});

		xml.node("register", [&] () {
			xml.attribute("name", "Fault Event Control");
			attribute_hex(xml, "value", read<Fault_event_control>());
			xml.attribute("mask", (bool)read<Fault_event_control::Mask>());
		});

		if (!read<Global_status::Rtps>())
			return;

		addr_t rt_addr = Root_table_address::Address::masked(read<Root_table_address>());

		xml.node("register", [&] () {
			xml.attribute("name", "Root Table Address");
			attribute_hex(xml, "value", rt_addr);
		});

		if (read<Root_table_address::Mode>() != Root_table_address::Mode::LEGACY) {
			error("Only supporting legacy translation mode");
			return;
		}

		/* dump root table, context table, and page tables */
		_report_helper.with_table<Root_table>(rt_addr,
			[&] (Root_table & root_table) {
				root_table.generate(xml, _env, _report_helper);
			});
	});
}


Intel::Io_mmu::Io_mmu(Env                      & env,
                      Io_mmu_devices           & io_mmu_devices,
                      Device::Name       const & name,
                      Device::Io_mem::Range      range,
                      Context_table_allocator  & table_allocator,
                      unsigned                   irq_number)
: Attached_mmio(env, range.start, range.size),
  Driver::Io_mmu(io_mmu_devices, name),
  _env(env),
  _managed_root_table(_env, table_allocator, *this, !coherent_page_walk()),
  _domain_allocator(_max_domains()-1)
{
	/* XXX support 3- and 5-level tables as well */
	if (!read<Capability::Sagaw_4_level>()) {
		error("IOMMU does not support 4-level page tables");
		return;
	}

	/* caches must be cleared if Esrtps is not set (see 6.6) */
	if (!read<Capability::Esrtps>())
		invalidate_all();
	else if (read<Global_status::Enabled>()) {
		error("IOMMU already enabled");
		return;
	}

	if (read<Capability::Rwbf>() && !read<Capability::Caching_mode>())
		warning("Requires explicit write-buffer flushing (not implemented)");

	/* enable fault event interrupts */
	if (irq_number) {
		_fault_irq.construct(_env, irq_number, 0, Irq_session::TYPE_MSI);

		_fault_irq->sigh(_fault_handler);
		_fault_irq->ack_irq();

		Irq_session::Info info = _fault_irq->info();
		if (info.type == Irq_session::Info::INVALID)
			error("Unable to enable fault event interrupts for ", name);
		else {
			write<Fault_event_address>((Fault_event_address::access_t)info.address);
			write<Fault_event_data>((Fault_event_data::access_t)info.value);
			write<Fault_event_control::Mask>(0);
		}
	}

	/* set root table address */
	write<Root_table_address>(
		Root_table_address::Address::masked(_managed_root_table.phys_addr()));

	/* issue set root table pointer command*/
	_global_command<Global_command::Srtp>(1);
}
