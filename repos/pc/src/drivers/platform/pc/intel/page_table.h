/*
 * \brief  x86_64 page table definitions
 * \author Adrian-Ken Rueegsegger
 * \author Johannes Schlatow
 * \date   2015-02-06
 */

/*
 * Copyright (C) 2015-2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVERS__PLATFORM__PC__INTEL__PAGE_TABLE_H_
#define _SRC__DRIVERS__PLATFORM__PC__INTEL__PAGE_TABLE_H_

#include <base/log.h>
#include <hw/assert.h>
#include <hw/page_flags.h>
#include <hw/util.h>
#include <util/misc_math.h>
#include <util/register.h>

#include <intel/report_helper.h>
#include <expanding_page_table_allocator.h>

namespace Intel {

	using namespace Hw;

	using size_t     = Genode::size_t;
	using addr_t     = Genode::addr_t;

	using Page_table_allocator =
		Driver::Expanding_page_table_allocator<4096>;

	/**
	 * (Generic) 4-level translation structures.
	 */

	enum {
		SIZE_LOG2_4KB   = 12,
		SIZE_LOG2_2MB   = 21,
		SIZE_LOG2_1GB   = 30,
		SIZE_LOG2_512GB = 39,
		SIZE_LOG2_256TB = 48,
	};

	class Level_1_translation_table;
	class Pml4_table;

	/**
	 * Page directory template.
	 *
	 * Page directories can refer to paging structures of the next higher level
	 * or directly map page frames by using large page mappings.
	 *
	 * \param PAGE_SIZE_LOG2  virtual address range size in log2
	 *                        of a single table entry
	 */
	template <typename ENTRY, unsigned PAGE_SIZE_LOG2>
	class Page_directory;

	/**
	 * Common descriptor.
	 *
	 * Table entry containing descriptor fields common to all levels.
	 */
	struct Common_descriptor : Genode::Register<64>
	{
		struct R   : Bitfield<0, 1> { };   /* read            */
		struct W   : Bitfield<1, 1> { };   /* write           */
		struct A   : Bitfield<8, 1> { };   /* accessed        */
		struct D   : Bitfield<9, 1> { };   /* dirty           */

		static bool present(access_t const v) { return R::get(v) || W::get(v); }

		static access_t create(Page_flags const &flags)
		{
			return R::bits(1)
				| W::bits(flags.writeable);
		}

		/**
		 * Return descriptor value with cleared accessed and dirty flags. These
		 * flags can be set by the MMU.
		 */
		static access_t clear_mmu_flags(access_t value)
		{
			A::clear(value);
			D::clear(value);
			return value;
		}
	};
}


class Intel::Level_1_translation_table
{
	private:

		static constexpr size_t PAGE_SIZE_LOG2 = SIZE_LOG2_4KB;
		static constexpr size_t MAX_ENTRIES    = 512;
		static constexpr size_t PAGE_SIZE      = 1UL << PAGE_SIZE_LOG2;
		static constexpr size_t PAGE_MASK      = ~((1UL << PAGE_SIZE_LOG2) - 1);

		class Misaligned {};
		class Invalid_range {};
		class Double_insertion {};

		struct Descriptor : Common_descriptor
		{
			using Common = Common_descriptor;

			struct Pa  : Bitfield<12, 36> { };        /* physical address     */

			static access_t create(Page_flags const &flags, addr_t const pa)
			{
				/* Ipat and Emt are ignored in legacy mode */

				return Common::create(flags)
					| Pa::masked(pa);
			}
		};

		typename Descriptor::access_t _entries[MAX_ENTRIES];

		struct Insert_func
		{
			Page_flags const    & flags;
			bool                  flush;

			Insert_func(Page_flags const & flags, bool flush)
			: flags(flags), flush(flush) { }

			void operator () (addr_t const vo, addr_t const pa,
			                  size_t const size,
			                  Descriptor::access_t &desc)
			{
				if ((vo & ~PAGE_MASK) || (pa & ~PAGE_MASK) ||
				    size < PAGE_SIZE)
				{
					throw Invalid_range();
				}
				Descriptor::access_t table_entry =
					Descriptor::create(flags, pa);

				if (Descriptor::present(desc) &&
				    Descriptor::clear_mmu_flags(desc) != table_entry)
				{
					throw Double_insertion();
				}
				desc = table_entry;

				if (flush)
					Utils::clflush(&desc);
			}
		};

		struct Remove_func
		{
			bool flush;

			Remove_func(bool flush) : flush(flush) { }

			void operator () (addr_t /* vo */, addr_t /* pa */, size_t /* size */,
			                  Descriptor::access_t &desc)
			{
				desc = 0;

				if (flush)
					Utils::clflush(&desc);
			}
		};

		template <typename FUNC>
		void _range_op(addr_t vo, addr_t pa, size_t size, FUNC &&func)
		{
			for (size_t i = vo >> PAGE_SIZE_LOG2; size > 0;
			     i = vo >> PAGE_SIZE_LOG2) {
				assert (i < MAX_ENTRIES);
				addr_t end = (vo + PAGE_SIZE) & PAGE_MASK;
				size_t sz  = Genode::min(size, end-vo);

				func(vo, pa, sz, _entries[i]);

				/* check whether we wrap */
				if (end < vo) return;

				size = size - sz;
				vo  += sz;
				pa  += sz;
			}
		}

	public:

		using Allocator = Page_table_allocator;

		static constexpr size_t MIN_PAGE_SIZE_LOG2 = SIZE_LOG2_4KB;
		static constexpr size_t ALIGNM_LOG2        = SIZE_LOG2_4KB;

		/**
		 * A page table consists of 512 entries that each maps a 4KB page
		 * frame. For further details refer to Intel SDM Vol. 3A, table 4-19.
		 */
		Level_1_translation_table()
		{
			if (!Hw::aligned(this, ALIGNM_LOG2)) throw Misaligned();
			Genode::memset(&_entries, 0, sizeof(_entries));
		}

		/**
		 * Returns True if table does not contain any page mappings.
		 */
		bool empty()
		{
			for (unsigned i = 0; i < MAX_ENTRIES; i++)
				if (Descriptor::present(_entries[i]))
					return false;
			return true;
		}

		void generate(Genode::Xml_generator & xml, Genode::Env &, Report_helper &)
		{
			using Genode::Hex;
			using Hex_str = Genode::String<20>;

			for (unsigned long i = 0; i < MAX_ENTRIES; i++) {
				if (Descriptor::present(_entries[i])) {
					xml.node("page", [&] () {
						addr_t addr = Descriptor::Pa::masked(_entries[i]);
						xml.attribute("index",   Hex_str(Hex(i << PAGE_SIZE_LOG2)));
						xml.attribute("value",   Hex_str(Hex(_entries[i])));
						xml.attribute("address", Hex_str(Hex(addr)));
						xml.attribute("accessed",(bool)Descriptor::A::get(_entries[i]));
						xml.attribute("dirty",   (bool)Descriptor::D::get(_entries[i]));
						xml.attribute("write",   (bool)Descriptor::W::get(_entries[i]));
						xml.attribute("read",    (bool)Descriptor::R::get(_entries[i]));
					});
				}
			}
		}

		/**
		 * Insert translations into this table
		 *
		 * \param vo     offset of the virtual region represented
		 *               by the translation within the virtual
		 *               region represented by this table
		 * \param pa     base of the physical backing store
		 * \param size   size of the translated region
		 * \param flags  mapping flags
		 * \param alloc  second level translation table allocator
		 */
		void insert_translation(addr_t vo, addr_t pa, size_t size,
		                        Page_flags const & flags, Allocator &,
		                        bool flush)
		{
			this->_range_op(vo, pa, size, Insert_func(flags, flush));
		}

		/**
		 * Remove translations that overlap with a given virtual region
		 *
		 * \param vo    region offset within the tables virtual region
		 * \param size  region size
		 * \param alloc second level translation table allocator
		 */
		void remove_translation(addr_t vo, size_t size, Allocator &, bool flush)
		{
			this->_range_op(vo, 0, size, Remove_func(flush));
		}

} __attribute__((aligned(1 << ALIGNM_LOG2)));


template <typename ENTRY, unsigned PAGE_SIZE_LOG2>
class Intel::Page_directory
{
	private:

		static constexpr size_t MAX_ENTRIES = 512;
		static constexpr size_t PAGE_SIZE   = 1UL << PAGE_SIZE_LOG2;
		static constexpr size_t PAGE_MASK   = ~((1UL << PAGE_SIZE_LOG2) - 1);

		using Allocator = Page_table_allocator;

		class Misaligned {};
		class Invalid_range {};
		class Double_insertion {};

		struct Base_descriptor : Common_descriptor
		{
			using Common = Common_descriptor;

			struct Ps : Common::template Bitfield<7, 1> { };  /* page size */

			static bool maps_page(access_t const v) { return Ps::get(v); }
		};

		struct Page_descriptor : Base_descriptor
		{
			using Base = Base_descriptor;

			/**
			 * Physical address
			 */
			struct Pa : Base::template Bitfield<PAGE_SIZE_LOG2,
			                                     48 - PAGE_SIZE_LOG2> { };


			static typename Base::access_t create(Page_flags const &flags,
			                                      addr_t const pa)
			{
				/* Ipat and Emt are ignored in legacy mode */

				return Base::create(flags)
				     | Base::Ps::bits(1)
				     | Pa::masked(pa);
			}
		};

		struct Table_descriptor : Base_descriptor
		{
			using Base = Base_descriptor;

			/**
			 * Physical address
			 */
			struct Pa : Base::template Bitfield<12, 36> { };

			static typename Base::access_t create(addr_t const pa)
			{
				static Page_flags flags { RW, NO_EXEC, USER, NO_GLOBAL,
				                          RAM, Genode::UNCACHED };
				return Base::create(flags) | Pa::masked(pa);
			}
		};

		typename Base_descriptor::access_t _entries[MAX_ENTRIES];

		struct Insert_func
		{
			using Descriptor = Base_descriptor;

			Page_flags const & flags;
			Allocator        & alloc;
			bool               flush;

			Insert_func(Page_flags const & flags, Allocator & alloc, bool flush)
			: flags(flags), alloc(alloc), flush(flush) { }

			void operator () (addr_t const vo, addr_t const pa,
			                  size_t const size,
			                  typename Descriptor::access_t &desc)
			{
				using Td = Table_descriptor;
				using access_t = typename Descriptor::access_t;

				/* can we insert a large page mapping? */
				if (!((vo & ~PAGE_MASK) || (pa & ~PAGE_MASK) ||
				      size < PAGE_SIZE))
				{
					access_t table_entry = Page_descriptor::create(flags, pa);

					if (Descriptor::present(desc) &&
					    Descriptor::clear_mmu_flags(desc) != table_entry) {
						throw Double_insertion(); }

					desc = table_entry;
					if (flush)
						Utils::clflush(&desc);
					return;
				}

				/* we need to use a next level table */
				if (!Descriptor::present(desc)) {

					/* create and link next level table */
					addr_t table_phys = alloc.construct<ENTRY>();
					desc = (access_t) Td::create(table_phys);

					if (flush)
						Utils::clflush(&desc);

				} else if (Descriptor::maps_page(desc)) {
					throw Double_insertion();
				}

				/* insert translation */
				alloc.with_table<ENTRY>(Td::Pa::masked(desc),
					[&] (ENTRY & table) {
						table.insert_translation(vo - (vo & PAGE_MASK), pa, size,
						                         flags, alloc, flush);
					},
					[&] () {
						error("Unable to get mapped table address for ", Hex(Td::Pa::masked(desc)));
					});
			}
		};

		struct Remove_func
		{
			Allocator & alloc;
			bool        flush;

			Remove_func(Allocator & alloc, bool flush)
			: alloc(alloc), flush(flush) { }

			void operator () (addr_t const vo, addr_t /* pa */,
			                  size_t const size,
			                  typename Base_descriptor::access_t &desc)
			{
				if (Base_descriptor::present(desc)) {
					if (Base_descriptor::maps_page(desc)) {
						desc = 0;
					} else {
						using Td = Table_descriptor;

						/* use allocator to retrieve virt address of table */
						addr_t  table_phys = Td::Pa::masked(desc);

						alloc.with_table<ENTRY>(table_phys,
							[&] (ENTRY & table) {
								addr_t const table_vo = vo - (vo & PAGE_MASK);
								table.remove_translation(table_vo, size, alloc, flush);
								if (table.empty()) {
									alloc.destruct<ENTRY>(table_phys);
									desc = 0;
								}
							},
							[&] () {
								error("Unable to get mapped table address for ", Hex(table_phys));
							});
					}

					if (desc == 0 && flush)
						Utils::clflush(&desc);
				}
			}
		};

		template <typename FUNC>
		void _range_op(addr_t vo, addr_t pa, size_t size, FUNC &&func)
		{
			for (size_t i = vo >> PAGE_SIZE_LOG2; size > 0;
			     i = vo >> PAGE_SIZE_LOG2)
			{
				assert (i < MAX_ENTRIES);
				addr_t end = (vo + PAGE_SIZE) & PAGE_MASK;
				size_t sz  = Genode::min(size, end-vo);

				func(vo, pa, sz, _entries[i]);

				/* check whether we wrap */
				if (end < vo) return;

				size = size - sz;
				vo  += sz;
				pa  += sz;
			}
		}

	public:

		static constexpr size_t MIN_PAGE_SIZE_LOG2 = SIZE_LOG2_4KB;
		static constexpr size_t ALIGNM_LOG2        = SIZE_LOG2_4KB;

		Page_directory()
		{
			if (!Hw::aligned(this, ALIGNM_LOG2)) throw Misaligned();
			Genode::memset(&_entries, 0, sizeof(_entries));
		}

		/**
		 * Returns True if table does not contain any page mappings.
		 *
		 * \return   false if an entry is present, True otherwise
		 */
		bool empty()
		{
			for (unsigned i = 0; i < MAX_ENTRIES; i++)
				if (Base_descriptor::present(_entries[i]))
					return false;
			return true;
		}

		void _generate_page_dir(unsigned long           index,
		                        Genode::Xml_generator & xml,
		                        Genode::Env           & env,
		                        Report_helper         & report_helper)
		{
			using Genode::Hex;
			using Hex_str = Genode::String<20>;

			xml.node("page_directory", [&] () {
				addr_t pd_addr = Table_descriptor::Pa::masked(_entries[index]);
				xml.attribute("index",   Hex_str(Hex(index << PAGE_SIZE_LOG2)));
				xml.attribute("value",   Hex_str(Hex(_entries[index])));
				xml.attribute("address", Hex_str(Hex(pd_addr)));

				report_helper.with_table<ENTRY>(pd_addr, [&] (ENTRY & pd) {
					/* dump page directory */
					pd.generate(xml, env, report_helper);
				});
			});
		}

		void _generate_page(unsigned long           index,
		                    Genode::Xml_generator & xml)
		{
			using Genode::Hex;
			using Hex_str = Genode::String<20>;

			xml.node("page", [&] () {
				addr_t addr = Page_descriptor::Pa::masked(_entries[index]);
				xml.attribute("index",   Hex_str(Hex(index << PAGE_SIZE_LOG2)));
				xml.attribute("value",   Hex_str(Hex(_entries[index])));
				xml.attribute("address", Hex_str(Hex(addr)));
				xml.attribute("accessed",(bool)Page_descriptor::A::get(_entries[index]));
				xml.attribute("dirty",   (bool)Page_descriptor::D::get(_entries[index]));
				xml.attribute("write",   (bool)Page_descriptor::W::get(_entries[index]));
				xml.attribute("read",    (bool)Page_descriptor::R::get(_entries[index]));
			});
		}

		void generate(Genode::Xml_generator & xml, Genode::Env & env, Report_helper & report_helper)
		{
			for (unsigned long i = 0; i < MAX_ENTRIES; i++)
				if (Base_descriptor::present(_entries[i])) {
					if (Base_descriptor::maps_page(_entries[i]))
						_generate_page(i, xml);
					else
						_generate_page_dir(i, xml, env, report_helper);
				}
		}

		/**
		 * Insert translations into this table
		 *
		 * \param vo     offset of the virtual region represented
		 *               by the translation within the virtual
		 *               region represented by this table
		 * \param pa     base of the physical backing store
		 * \param size   size of the translated region
		 * \param flags  mapping flags
		 * \param alloc  second level translation table allocator
		 */
		void insert_translation(addr_t vo, addr_t pa, size_t size,
		                        Page_flags const & flags,
		                        Allocator & alloc, bool flush) {
			_range_op(vo, pa, size, Insert_func(flags, alloc, flush)); }

		/**
		 * Remove translations that overlap with a given virtual region
		 *
		 * \param vo    region offset within the tables virtual region
		 * \param size  region size
		 * \param alloc second level translation table allocator
		 */
		void remove_translation(addr_t vo, size_t size, Allocator & alloc,
		                        bool flush) {
			_range_op(vo, 0, size, Remove_func(alloc, flush)); }
};


namespace Intel {

	struct Level_2_translation_table
	:
		Page_directory< Level_1_translation_table, SIZE_LOG2_2MB>
	{ } __attribute__((aligned(1 << ALIGNM_LOG2)));

	struct Level_3_translation_table
	:
		Page_directory<Level_2_translation_table, SIZE_LOG2_1GB>
	{ } __attribute__((aligned(1 << ALIGNM_LOG2)));

}


class Intel::Pml4_table
{
	public:

		using Allocator = Page_table_allocator;

	private:

		static constexpr size_t PAGE_SIZE_LOG2 = SIZE_LOG2_512GB;
		static constexpr size_t SIZE_LOG2      = SIZE_LOG2_256TB;
		static constexpr size_t SIZE_MASK      = (1UL << SIZE_LOG2) - 1;
		static constexpr size_t MAX_ENTRIES    = 512;
		static constexpr size_t PAGE_SIZE      = 1UL << PAGE_SIZE_LOG2;
		static constexpr size_t PAGE_MASK      = ~((1UL << PAGE_SIZE_LOG2) - 1);

		class Misaligned {};
		class Invalid_range {};

		struct Descriptor : Common_descriptor
		{
			struct Pa  : Bitfield<12, SIZE_LOG2> { };    /* physical address */

			static access_t create(addr_t const pa)
			{
				static Page_flags flags { RW, NO_EXEC, USER, NO_GLOBAL,
				                          RAM, Genode::UNCACHED };
				return Common_descriptor::create(flags) | Pa::masked(pa);
			}
		};

		typename Descriptor::access_t _entries[MAX_ENTRIES];

		using ENTRY = Level_3_translation_table;

		struct Insert_func
		{
			Page_flags const & flags;
			Allocator        & alloc;
			bool               flush;

			Insert_func(Page_flags const & flags,
			            Allocator & alloc, bool flush)
			: flags(flags), alloc(alloc), flush(flush) { }

			void operator () (addr_t const vo, addr_t const pa,
			                  size_t const size,
			                  Descriptor::access_t &desc)
			{
				/* we need to use a next level table */
				if (!Descriptor::present(desc)) {
					/* create and link next level table */
					addr_t table_phys = alloc.construct<ENTRY>();
					desc = Descriptor::create(table_phys);

					if (flush)
						Utils::clflush(&desc);
				}

				/* insert translation */
				addr_t table_phys = Descriptor::Pa::masked(desc);
				alloc.with_table<ENTRY>(table_phys,
					[&] (ENTRY & table) {
						addr_t const table_vo = vo - (vo & PAGE_MASK);
						table.insert_translation(table_vo, pa, size, flags, alloc, flush);
					},
					[&] () {
						error("Unable to get mapped table address for ", Hex(table_phys));
					});
			}
		};

		struct Remove_func
		{
			Allocator & alloc;
			bool        flush;

			Remove_func(Allocator & alloc, bool flush)
			: alloc(alloc), flush(flush) { }

			void operator () (addr_t const vo, addr_t /* pa */,
			                  size_t const size,
			                  Descriptor::access_t &desc)
			{
				if (Descriptor::present(desc)) {
					/* use allocator to retrieve virt address of table */
					addr_t  table_phys = Descriptor::Pa::masked(desc);
					alloc.with_table<ENTRY>(table_phys,
						[&] (ENTRY & table) {
							addr_t const table_vo = vo - (vo & PAGE_MASK);
							table.remove_translation(table_vo, size, alloc, flush);
							if (table.empty()) {
								alloc.destruct<ENTRY>(table_phys);
								desc = 0;

								if (flush)
									Utils::clflush(&desc);
							}
						},
						[&] () {
							error("Unable to get mapped table address for ", Hex(table_phys));
						});
				}
			}
		};

		template <typename FUNC>
		void _range_op(addr_t vo, addr_t pa, size_t size, FUNC &&func)
		{
			for (size_t i = (vo & SIZE_MASK) >> PAGE_SIZE_LOG2; size > 0;
			     i = (vo & SIZE_MASK) >> PAGE_SIZE_LOG2) {
				assert (i < MAX_ENTRIES);
				addr_t end = (vo + PAGE_SIZE) & PAGE_MASK;
				size_t sz  = Genode::min(size, end-vo);

				func(vo, pa, sz, _entries[i]);

				/* check whether we wrap */
				if (end < vo) return;

				size = size - sz;
				vo  += sz;
				pa  += sz;
			}
		}

	protected:

		/**
		 * Return how many entries of an alignment fit into region
		 */
		static constexpr size_t _count(size_t region, size_t alignment)
		{
			return Genode::align_addr<size_t>(region, (int)alignment)
			       / (1UL << alignment);
		}

	public:

		static constexpr size_t MIN_PAGE_SIZE_LOG2 = SIZE_LOG2_4KB;
		static constexpr size_t ALIGNM_LOG2        = SIZE_LOG2_4KB;

		Pml4_table()
		{
			if (!Hw::aligned(this, ALIGNM_LOG2)) throw Misaligned();
			Genode::memset(&_entries, 0, sizeof(_entries));
		}

		explicit Pml4_table(Pml4_table & kernel_table) : Pml4_table()
		{
			static size_t first = (0xffffffc000000000 & SIZE_MASK) >> PAGE_SIZE_LOG2;
			for (size_t i = first; i < MAX_ENTRIES; i++)
				_entries[i] = kernel_table._entries[i];
		}

		/**
		 * Returns True if table does not contain any page mappings.
		 *
		 * \return  false if an entry is present, True otherwise
		 */
		bool empty()
		{
			for (unsigned i = 0; i < MAX_ENTRIES; i++)
				if (Descriptor::present(_entries[i]))
					return false;
			return true;
		}

		void generate(Genode::Xml_generator & xml, Genode::Env & env, Report_helper & report_helper)
		{
			using Genode::Hex;
			using Hex_str = Genode::String<20>;

			for (unsigned long i = 0; i < MAX_ENTRIES; i++)
				if (Descriptor::present(_entries[i])) {
					xml.node("level4_entry", [&] () {
						addr_t level3_addr = Descriptor::Pa::masked(_entries[i]);
						xml.attribute("index",   Hex_str(Hex(i << PAGE_SIZE_LOG2)));
						xml.attribute("value",   Hex_str(Hex(_entries[i])));
						xml.attribute("address", Hex_str(Hex(level3_addr)));


						report_helper.with_table<ENTRY>(level3_addr, [&] (ENTRY & level3_table) {
							/* dump stage2 table */
							level3_table.generate(xml, env, report_helper);
						});
					});
				}
		}

		/**
		 * Insert translations into this table
		 *
		 * \param vo     offset of the virtual region represented
		 *               by the translation within the virtual
		 *               region represented by this table
		 * \param pa     base of the physical backing store
		 * \param size   size of the translated region
		 * \param flags  mapping flags
		 * \param alloc  second level translation table allocator
		 */
		void insert_translation(addr_t vo, addr_t pa, size_t size,
		                        Page_flags const & flags, Allocator & alloc,
		                        bool flush) {
			_range_op(vo, pa, size, Insert_func(flags, alloc, flush)); }

		/**
		 * Remove translations that overlap with a given virtual region
		 *
		 * \param vo    region offset within the tables virtual region
		 * \param size  region size
		 * \param alloc second level translation table allocator
		 */
		void remove_translation(addr_t vo, size_t size, Allocator & alloc,
		                        bool flush)
		{
			_range_op(vo, 0, size, Remove_func(alloc, flush));
		}

} __attribute__((aligned(1 << ALIGNM_LOG2)));

#endif /* _SRC__DRIVERS__PLATFORM__PC__INTEL__PAGE_TABLE_H_ */
