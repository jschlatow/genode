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

#ifndef _SRC__DRIVERS__PLATFORM__INTEL__IO_MMU_H_
#define _SRC__DRIVERS__PLATFORM__INTEL__IO_MMU_H_

/* Genode includes */
#include <os/attached_mmio.h>
#include <util/register_set.h>
#include <base/allocator.h>
#include <base/attached_ram_dataspace.h>

/* Platform-driver includes */
#include <device.h>
#include <io_mmu.h>
#include <dma_allocator.h>

/* local includes */
#include <intel/managed_root_table.h>
#include <intel/report_helper.h>
#include <intel/page_table.h>
#include <intel/domain_allocator.h>
#include <expanding_page_table_allocator.h>

namespace Intel {
	using namespace Genode;
	using namespace Driver;

	using Context_table_allocator = Managed_root_table::Allocator;

	class Io_mmu;
	class Io_mmu_factory;
}


class Intel::Io_mmu : private Attached_mmio,
                      public  Driver::Io_mmu,
                      private Translation_table_registry
{
	public:

		/* Use derived domain class to store reference to buffer registry */
		class Domain : public Driver::Io_mmu::Domain,
		               public Registered_translation_table
		{
			private:

				using Table_allocator   = Expanding_page_table_allocator<4096>;

				Intel::Io_mmu & _intel_iommu;
				Env           & _env;
				Ram_allocator & _ram_alloc;

				Table_allocator    _table_allocator;
				Domain_allocator & _domain_allocator;
				Domain_id          _domain_id         { _domain_allocator.alloc() };

				addr_t             _translation_table_phys {
					_table_allocator.construct<Pml4_table>() };

				Pml4_table       & _translation_table {
					*(Pml4_table*)virt_addr(_translation_table_phys) };

			public:

				void enable_pci_device(Pci::Bdf const) override;
				void disable_pci_device(Pci::Bdf const) override;

				void add_range(Range const &,
				               addr_t const,
				               Dataspace_capability const) override;
				void remove_range(Range const &) override;

				/* Registered_translation_table interface */
				virtual addr_t virt_addr(addr_t phys_addr) override
				{
					addr_t va { 0 };

					_table_allocator.with_table<Pml4_table>(phys_addr,
						[&] (Pml4_table & table) { va = (addr_t)&table; },
						[&] ()                   { va = 0; });

					return va;
				}

				Domain(Intel::Io_mmu              & intel_iommu,
				       Allocator                  & md_alloc,
				       Registry<Dma_buffer> const & buffer_registry,
				       Env                        & env,
				       Ram_allocator              & ram_alloc,
				       Domain_allocator           & domain_allocator)
				: Driver::Io_mmu::Domain(intel_iommu, md_alloc, buffer_registry),
				  Registered_translation_table(intel_iommu),
				  _intel_iommu(intel_iommu),
				  _env(env),
				  _ram_alloc(ram_alloc),
				  _table_allocator(_env, md_alloc, ram_alloc, 2),
				  _domain_allocator(domain_allocator)
				{
					buffer_registry.for_each([&] (Dma_buffer const & buf) {
						add_range({ buf.dma_addr, buf.size }, buf.phys_addr, buf.cap); });
				}

				~Domain() override
				{
					_intel_iommu.root_table().remove_context(_translation_table_phys);

					_intel_iommu.invalidate_all(_domain_id);

					_domain_allocator.free(_domain_id);
				}
		};

	private:

		Env                & _env;

		/**
		 * For a start, we keep a distinct root table for every hardware unit.
		 *
		 * This doubles RAM requirements for allocating page tables when devices
		 * in the scope of different hardware units are used in the same session,
		 * yet simplifies the implementation. In order to use a single root table
		 * for all hardware units, we'd need to have a single Io_mmu object
		 * controlling all hardware units. Otherwise, the session component will
		 * create separate Domain objects that receive identical modification
		 * instructions.
		 */
		bool                          _verbose            { true };
		Managed_root_table            _managed_root_table;
		Report_helper                 _report_helper      { *this };
		Domain_allocator              _domain_allocator;
		Constructible<Irq_connection> _fault_irq          { };
		Signal_handler<Io_mmu>        _fault_handler      {
			_env.ep(), *this, &Io_mmu::_handle_faults };

		/**
		 * Registers
		 */

		struct Version : Register<0x0, 32>
		{
			struct Minor : Bitfield<0, 4> { };
			struct Major : Bitfield<4, 4> { };
		};

		struct Capability : Register<0x8, 64>
		{
			/* enhanced set root table pointer support */
			struct Esrtps : Bitfield<63,1> { };

			/* number of fault-recording registers (n-1) */
			struct Nfr : Bitfield<40,8> { };

			/* fault recording register offset */
			struct Fro : Bitfield<24,10> { };

			struct Sagaw_5_level : Bitfield<11,1> { };
			struct Sagaw_4_level : Bitfield<10,1> { };
			struct Sagaw_3_level : Bitfield< 9,1> { };

			struct Caching_mode : Bitfield<7,1> { };

			struct Rwbf    : Bitfield<4,1> { };

			struct Domains : Bitfield<0,3> { };

		};

		struct Extended_capability : Register<0x10, 64>
		{
			/* IOTLB register offset */
			struct Iro : Bitfield<8,10> { };

			struct Page_walk_coherency : Bitfield<0,1> { };
		};

		struct Global_command : Register<0x18, 32>
		{
			struct Enable : Bitfield<31,1> { };

			/* set root table pointer */
			struct Srtp   : Bitfield<30,1> { };

			/* set interrupt remap table pointer */
			struct Sirtp   : Bitfield<24,1> { };
		};

		struct Global_status : Register<0x1c, 32>
		{
			struct Enabled : Bitfield<31,1> { };

			/* root table pointer status */
			struct Rtps  : Bitfield<30,1> { };

			/* queued invalidation enable status */
			struct Qies  : Bitfield<26,1> { };

			/* interrupt remapping table pointer status */
			struct Irtps : Bitfield<24,1> { };
		};

		struct Root_table_address : Register<0x20, 64>
		{
			struct Mode    : Bitfield<10, 2> { enum { LEGACY = 0x00 }; };
			struct Address : Bitfield<12,52> { };
		};

		struct Context_command : Register<0x28, 64>
		{
			struct Invalidate : Bitfield<63,1> { };

			/* invalidation request granularity */
			struct Cirg       : Bitfield<61,2> {
				enum {
					GLOBAL = 0x1,
					DOMAIN = 0x2,
					DEVICE = 0x3
				};
			};

			/* actual invalidation granularity */
			struct Caig      : Bitfield<59,2> { };

			/* source id */
			struct Sid       : Bitfield<16,16> { };

			/* domain id */
			struct Did       : Bitfield<0,16> { };
		};

		struct Fault_status : Register<0x34, 32>
		{
			/* fault record index */
			struct Fri : Bitfield<8,8> { };

			/* invalidation queue error */
			struct Iqe : Bitfield<4,1> { };

			/* primary pending fault */
			struct Pending : Bitfield<1,1> { };

			/* primary fault overflow */
			struct Overflow : Bitfield<0,1> { };
		};

		struct Fault_event_control : Register<0x38, 32>
		{
			struct Mask : Bitfield<31,1> { };
		};

		struct Fault_event_data : Register<0x3c, 32>
		{ };

		struct Fault_event_address : Register<0x40, 32>
		{ };

		/* IOTLB registers may be at offsets 0 to 1024*16 */
		struct All_registers : Register_array<0x0, 64, 256, 64>
		{ };

		struct Fault_record_hi : Genode::Register<64>
		{
			static unsigned offset() { return 1; }

			struct Fault  : Bitfield<63,1> { };
			struct Type1  : Bitfield<62,1> { };

			/* address type */
			struct At     : Bitfield<60,2> { };

			struct Pasid  : Bitfield<40,10> { };
			struct Reason : Bitfield<32, 8> { };

			/* PASID present */
			struct Pp     : Bitfield<31,1> { };

			/* execute permission requested */
			struct Exe    : Bitfield<30,1> { };

			/* privilege mode requested */
			struct Priv   : Bitfield<29,1> { };
			struct Type2  : Bitfield<28,1> { };
			struct Source : Bitfield<0,16> { };

			struct Type   : Bitset_2<Type1, Type2> {
				enum {
					WRITE_REQUEST  = 0x0,
					READ_REQUEST   = 0x1,
					PAGE_REQUEST   = 0x2,
					ATOMIC_REQUEST = 0x3
				};
			};
		};

		struct Fault_record_lo : Genode::Register<64>
		{
			static unsigned offset() { return 0; }

			struct Info : Bitfield<12,52> { };
		};

		struct Iotlb : Genode::Register<64>
		{
			struct Invalidate : Bitfield<63,1> { };

			/* IOTLB invalidation request granularity */
			struct Iirg : Bitfield<60,2> {
				enum {
					GLOBAL = 0x1,
					DOMAIN = 0x2,
					DEVICE = 0x3
				};
			};

			/* IOTLB actual invalidation granularity */
			struct Iaig : Bitfield<57,2> { };

			/* drain reads/writes */
			struct Dr   : Bitfield<49,1> { };
			struct Dw   : Bitfield<48,1> { };

			/* domain id */
			struct Did  : Bitfield<32,16> { };

		};
		
		uint32_t _max_domains() {
			return 1 << (4 + read<Capability::Domains>()*2); }

		template <typename BIT>
		void _global_command(bool set)
		{
			Global_status::access_t  status = read<Global_status>();
			Global_command::access_t cmd    = status;

			/* keep status bits but clear one-shot bits */
			Global_command::Srtp::clear(cmd);
			Global_command::Sirtp::clear(cmd);

			if (set) {
				BIT::set(cmd);
				BIT::set(status);
			} else {
				BIT::clear(cmd);
				BIT::clear(status);
			}

			/* write command */
			write<Global_command>(cmd);

			/* wait until command completed */
			while (read<Global_status>() != status);
		}

		template <typename BITFIELD>
		unsigned long _offset()
		{
			/* BITFIELD denotes registers offset counting 128-bit as one register */
			unsigned offset = read<BITFIELD>();

			/* return 64-bit register offset */
			return offset*2;
		}

		template <typename OFFSET_BITFIELD>
		void write_offset_register(unsigned index, All_registers::access_t value) {
			write<All_registers>(value, _offset<OFFSET_BITFIELD>() + index); }

		template <typename OFFSET_BITFIELD>
		All_registers::access_t read_offset_register(unsigned index) {
			return read<All_registers>(_offset<OFFSET_BITFIELD>() + index); }

		void write_iotlb_reg(Iotlb::access_t v) {
			write_offset_register<Extended_capability::Iro>(1, v); }

		Iotlb::access_t read_iotlb_reg() {
			return read_offset_register<Extended_capability::Iro>(1); }

		template <typename REG>
		REG::access_t read_fault_record(unsigned index) {
			return read_offset_register<Capability::Fro>(index*2 + REG::offset()); }

		void clear_fault_record(unsigned index) {
			write_offset_register<Capability::Fro>(index*2 + Fault_record_hi::offset(),
			                                       Fault_record_hi::Fault::bits(1));
		}

		void _handle_faults();

		/**
		 * Io_mmu interface
		 */

		void _enable() override {
			_global_command<Global_command::Enable>(1);

			if (_verbose)
				log("enabled IOMMU ", name());
		}

		void _disable() override
		{
			_global_command<Global_command::Enable>(0);

			if (_verbose)
				log("disabled IOMMU ", name());
		}

	public:

		Managed_root_table & root_table() { return _managed_root_table; }

		void generate(Xml_generator &) override;

		void invalidate_iotlb(Domain_id, addr_t, size_t);
		void invalidate_all(Domain_id domain = Domain_id { Domain_id::INVALID }, Pci::rid_t = 0);

		bool coherent_page_walk() { return read<Extended_capability::Page_walk_coherency>(); }
		bool caching_mode()       { return read<Capability::Caching_mode>(); }

		/**
		 * Io_mmu interface
		 */

		Driver::Io_mmu::Domain & create_domain(Allocator     & md_alloc,
		                                       Ram_allocator & ram_alloc,
		                                       Registry<Dma_buffer> const & buffer_registry,
		                                       Ram_quota_guard &,
		                                       Cap_quota_guard &) override
		{
			return *new (md_alloc) Intel::Io_mmu::Domain(*this,
			                                             md_alloc,
			                                             buffer_registry,
			                                             _env,
			                                             ram_alloc,
			                                             _domain_allocator);
		}

		/**
		 * Constructor/Destructor
		 */

		Io_mmu(Env                      & env,
		       Io_mmu_devices           & io_mmu_devices,
		       Device::Name       const & name,
		       Device::Io_mem::Range      range,
		       Context_table_allocator  & table_allocator,
		       unsigned                   irq_number);

		~Io_mmu() { _destroy_domains(); }
};


class Intel::Io_mmu_factory : public Driver::Io_mmu_factory
{
	private:

		using Table_array     = Context_table_allocator::Array<510>;

		Genode::Env  & _env;

		/* Allocate 2MB RAM for root table and 256 context tables */
		Attached_ram_dataspace    _allocator_ds    { _env.ram(),
		                                             _env.rm(),
		                                             2*1024*1024,
		                                             Cache::CACHED };

		/* add page-table allocator array at _allocator_ds.local_addr() */
		Table_array             & _table_array     { *Genode::construct_at<Table_array>(
			_allocator_ds.local_addr<void>(),
			[&] (void *) {
				return _env.pd().dma_addr(_allocator_ds.cap());
			})};

		/* We use a single allocator for context tables for all IOMMU devices */
		Context_table_allocator & _table_allocator { _table_array.alloc() };

	public:

		Io_mmu_factory(Genode::Env & env, Registry<Driver::Io_mmu_factory> & registry)
		: Driver::Io_mmu_factory(registry, Device::Type { "intel_iommu" }),
		  _env(env)
		{ }

		void create(Allocator & alloc, Io_mmu_devices & io_mmu_devices, Device const & device) override
		{
			using Range = Device::Io_mem::Range;

			unsigned irq_number { 0 };
			device.for_each_irq([&] (unsigned idx, unsigned nbr, Irq_session::Type,
			                         Irq_session::Polarity, Irq_session::Trigger, bool)
			{
				if (idx == 0)
					irq_number = nbr;
			});

			device.for_each_io_mem([&] (unsigned idx, Range range, Device::Pci_bar, bool)
			{
				if (idx == 0)
					new (alloc) Intel::Io_mmu(_env, io_mmu_devices, device.name(),
					                          range, _table_allocator, irq_number);
			});
		}
};

#endif /* _SRC__DRIVERS__PLATFORM__INTEL__IO_MMU_H_ */
