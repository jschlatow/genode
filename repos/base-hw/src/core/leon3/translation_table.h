/*
 * \brief   LEON3 translation table for core
 * \author  Johannes Schlatow
 * \date    2014-07-24
 *
 * TODO implement LEON3 MMU
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _LEON3__TRANSLATION_TABLE_H_
#define _LEON3__TRANSLATION_TABLE_H_

/* Genode includes */
#include <util/register.h>

/* base-hw includes */
#include <page_flags.h>
#include <page_slab.h>

namespace Leon3
{
	using namespace Genode;

	/**
	 * Check if 'p' is aligned to 1 << 'alignm_log2'
	 */
	inline bool aligned(addr_t const a, size_t const alignm_log2) {
		return a == ((a >> alignm_log2) << alignm_log2); }

	/**
	 * Return permission configuration according to given mapping flags
	 *
	 * \param T      targeted translation-table-descriptor type
	 * \param flags  mapping flags
	 *
	 * \return  descriptor value with AP and XN set and the rest left zero
	 */
	template <typename T>
	static typename T::access_t
	access_permission_bits(Page_flags const & flags)
	{
		typedef typename T::Acc Acc;
		typedef typename T::access_t access_t;
		bool const w = flags.writeable;
		bool const x = flags.executable;
		bool const p = flags.privileged;
		access_t ap;
		if (w & x) { if (p) { ap = Acc::bits(Acc::SRWX); }
		             else   { ap = Acc::bits(Acc::RWX); }
		} else if (w) { if (p) { ap = Acc::bits(Acc::SRW_URO); }
		                else   { ap = Acc::bits(Acc::RW); }
		} else if (x) { if (p) { ap = Acc::bits(Acc::SRX); }
		                else   { ap = Acc::bits(Acc::RX); }
		} else { if (p) { ap = Acc::bits(Acc::SRX); }
		         else   { ap = Acc::bits(Acc::RO); }
		}
		return ap;
	}

	/**
	 * Memory region attributes for the translation descriptor 'T'
	 */
	template <typename T>
	static typename T::access_t
	memory_region_attr(Page_flags const & flags);

	/***************************
	 ** Exception definitions **
	 ***************************/

	class Double_insertion {};
	class Misaligned {};
	class Invalid_range {};

	template <unsigned MSB, unsigned LSB>
	class Page_table
	{
		public:

			enum {
				SIZE_LOG2   = (MSB-LSB+1) + 2,
				SIZE        = 1 << SIZE_LOG2,
				ALIGNM_LOG2 = SIZE_LOG2,
			};

			/**
			 * Common descriptor structure
			 */
			struct Descriptor : Register<32>
			{
				enum {
					VIRT_SIZE_LOG2   = LSB,
					VIRT_SIZE        = 1 << VIRT_SIZE_LOG2,
					VIRT_OFFSET_MASK = (1 << VIRT_SIZE_LOG2) - 1,
					VIRT_BASE_MASK   = ~(VIRT_OFFSET_MASK),
				};

				struct Entry_type : Bitfield<0, 2>
				{
					enum Type {
						INVALID    = 0x0,
						PAGE_TABLE = 0x1,
						PAGE       = 0x2,
						RESERVED   = 0x3
					};
				};

				/**
				 * Descriptor types
				 */
				typedef typename Entry_type::Type Type;

				/**
				 * Get descriptor type of 'v'
				 */
				static Type type(access_t const v)
				{
					return (Type)Entry_type::get(v);
				}

				/**
				 * Set descriptor type of 'v'
				 */
				static void type(access_t & v, Type const t)
				{
					Entry_type::set(v, t);
				}

				/**
				 * Invalidate descriptor 'v'
				 */
				static void invalidate(access_t & v) { type(v, Type::INVALID); }

				/**
				 * Return if descriptor 'v' is valid
				 */
				static bool valid(access_t & v) {
					return type(v) != Type::INVALID; }

				static inline Type align(addr_t vo, addr_t pa, size_t size)
				{
					return ((vo & VIRT_OFFSET_MASK) || (pa & VIRT_OFFSET_MASK) ||
							  size < VIRT_SIZE) ? Type::PAGE_TABLE : Type::PAGE;
				}
			};

			/**
			 * Section translation descriptor
			 */
			struct Page_table_entry : Descriptor
			{
				typedef typename Descriptor::access_t access_t;

				enum {
					PPN_SHIFT = 4
				};

				struct Acc    : Descriptor::template Bitfield<2, 3>            /* access permission */
				{
					enum {
						RO  = 0x0,
						RW  = 0x1,
						RX  = 0x2,
						RWX = 0x3,
						XO  = 0x4,
						SRW_URO = 0x5,
						SRX     = 0x6,
						SRWX    = 0x7,
					};
				};
				struct R      : Descriptor::template Bitfield<5, 1>  { };      /* referenced           */
				struct M      : Descriptor::template Bitfield<6, 1>  { };      /* modified             */
				struct C      : Descriptor::template Bitfield<7, 1>  { };      /* cacheable            */
				struct Ppn    : Descriptor::template Bitfield<8, 24> { };      /* physical page number */

				/**
				 * Compose descriptor value
				 */
				static access_t create(Page_flags const & flags,
											  addr_t const pa)
				{
					access_t v = access_permission_bits<Page_table_entry>(flags);
					v |= C::bits(flags.cacheable);
					v |= Ppn::masked(pa >> PPN_SHIFT);
					Descriptor::type(v, Descriptor::Type::PAGE);
					return v;
				}
			};

		protected:

			/*
			 * Table payload
			 *
			 * Must be the only member of this class
			 */
			typename Descriptor::access_t _entries[SIZE/sizeof(typename Descriptor::access_t)];

			enum { MAX_INDEX = sizeof(_entries) / sizeof(_entries[0]) - 1 };

			/**
			 * Get entry index by virtual offset
			 *
			 * \param i   is overridden with the index if call returns true
			 * \param vo  virtual offset relative to the virtual table base
			 *
			 * \retval  true   on success
			 * \retval  false  translation failed
			 */
			bool _index_by_vo (unsigned & i, addr_t const vo) const
			{
				if (vo > max_virt_offset()) return false;
				i = vo >> Descriptor::VIRT_SIZE_LOG2;
				return true;
			}

		public:

			/**
			 * Constructor
			 */
			Page_table()
			{
				if (!aligned((addr_t)this, ALIGNM_LOG2))
					throw Misaligned();

				/* start with an empty table */
				memset(&_entries, 0, sizeof(_entries));
			}

			/**
			 * Maximum virtual offset that can be translated by this table
			 */
			static addr_t max_virt_offset()
			{
				return (MAX_INDEX << Descriptor::VIRT_SIZE_LOG2)
						  + (Descriptor::VIRT_SIZE - 1);
			}

			/**
			 * Insert one atomic translation into this table
			 *
			 * \param vo    offset of the virtual region represented
			 *              by the translation within the virtual
			 *              region represented by this table
			 * \param pa    base of the physical backing store
			 * \param size  size of the translated region
			 * \param flags mapping flags
			 *
			 * This method overrides an existing translation in case
			 * that it spans the the same virtual range otherwise it
			 * throws a Double_insertion.
			 */
			void insert_translation(addr_t vo,
											addr_t pa,
											size_t size,
											Page_flags const & flags,
											Page_slab *)
			{
				constexpr size_t sz = Descriptor::VIRT_SIZE;

				for (unsigned i; (size > 0) && _index_by_vo (i, vo);
					 size = (size < sz) ? 0 : size - sz, vo += sz, pa += sz) {

					if (Descriptor::valid(_entries[i]) &&
						_entries[i] != Page_table_entry::create(flags, pa))
						throw Double_insertion();

					/* compose new descriptor value */
					_entries[i] = Page_table_entry::create(flags, pa);
				}
			}

			/**
			 * Remove translations that overlap with a given virtual region
			 *
			 * \param vo    region offset within the tables virtual region
			 * \param size  region size
			 */
			void remove_translation(addr_t vo, size_t size, Page_slab *)
			{
				constexpr size_t sz = Descriptor::VIRT_SIZE;

				for (unsigned i; (size > 0) && _index_by_vo(i, vo);
					  size = (size < sz) ? 0 : size - sz, vo += sz) {

					switch (Descriptor::type(_entries[i])) {
						case Descriptor::Type::PAGE:
						{
							Descriptor::invalidate(_entries[i]);
						}
						default: ;
					}
				}
			}

			/**
			 * Does this table solely contain invalid entries
			 */
			bool empty()
			{
				for (unsigned i = 0; i <= MAX_INDEX; i++)
					if (Descriptor::valid(_entries[i])) return false;
				return true;
			}

	};

	template <unsigned MSB, unsigned LSB, typename NEXT_LVL>
	class Multi_level_page_table : public Page_table<MSB, LSB>
	{
		friend class Context_table;

		public: 
			typedef          Page_table<MSB, LSB>       Parent;
			typedef typename Parent::Descriptor         Descriptor;
			typedef typename Parent::Page_table_entry   Page_table_entry;

			typedef NEXT_LVL Next_level_page_table;

		protected:

			/**
			 * Link to a next-level translation table
			 */
			struct Page_table_descriptor : Descriptor
			{
				typedef typename Descriptor::access_t access_t;

				enum {
					PTP_SHIFT = 4
				};

				struct Ptp    : Descriptor::template Bitfield<2, 30> { };   /* page table pointer  */

				static Next_level_page_table *page_table(access_t &v)
				{
					return (Next_level_page_table*)(Ptp::masked(v) << PTP_SHIFT);
				}

				/**
				 * Compose descriptor value
				 */
				static access_t create(Next_level_page_table * const pt)
				{
						access_t v = Ptp::masked((addr_t)pt >> PTP_SHIFT);
						Descriptor::type(v, Descriptor::Type::PAGE_TABLE);
						return v;
				}
			};


			/**
			 * Insert a second level translation at the given entry index
			 *
			 * \param i     the related index
			 * \param vo    virtual start address of range
			 * \param pa    physical start address of range
			 * \param size  size of range
			 * \param flags mapping flags
			 * \param slab  second level page slab allocator
			 */
			void _insert_next_level(unsigned i,
										  addr_t const vo,
										  addr_t const pa,
										  size_t const size,
										  Page_flags const & flags,
										  Page_slab * slab)
			{

				typedef Page_table_descriptor Ptd;
				typedef Next_level_page_table Pt;

				Pt *pt = 0;
				switch (Descriptor::type(Parent::_entries[i])) {

					case Descriptor::Type::INVALID:
					{
						if (!slab) throw Allocator::Out_of_memory();

						/* create and link page table */
						pt = new (slab) Pt();
						Pt *pt_phys = (Pt*) slab->phys_addr(pt);
						pt_phys = pt_phys ? pt_phys : pt; /* hack for core */
						Parent::_entries[i] = Page_table_descriptor::create(pt_phys);
					}

					case Descriptor::Type::PAGE_TABLE:
					{
						/* use allocator to retrieve virtual address of page table */
						Pt *pt_phys = Page_table_descriptor::page_table(Parent::_entries[i]);
						pt = (Pt *) slab->virt_addr(pt_phys);
						pt = pt ? pt : pt_phys ; /* hack for core */
						break;
					}

					default:
					{
						throw Double_insertion();
					}
				};

				/* insert translation */
				pt->insert_translation(vo - (vo & Descriptor::VIRT_BASE_MASK),   /* remove anything above LSB */
											  pa, size, flags, slab);
			}
		public:

			/**
			 * Placement new
			 */
			void * operator new (size_t, void * p) { return p; }

			/**
			 * Insert translations into this table
			 *
			 * \param vo           offset of the virtual region represented
			 *                     by the translation within the virtual
			 *                     region represented by this table
			 * \param pa           base of the physical backing store
			 * \param size         size of the translated region
			 * \param flags        mapping flags
			 * \param slab  second level page slab allocator
			 */
			void insert_translation(addr_t vo,
											addr_t pa,
											size_t size,
											Page_flags const & flags,
											Page_slab * slab)
			{
				for (unsigned i; (size > 0) && Parent::_index_by_vo (i, vo);) {

					addr_t end = (vo + Descriptor::VIRT_SIZE)
									 & Descriptor::VIRT_BASE_MASK;

					/* decide granularity of entry that can be inserted */
					switch (Descriptor::align(vo, pa, size)) {
						case Descriptor::Type::PAGE:
						{
							if (Descriptor::valid(Parent::_entries[i]) &&
								 Parent::_entries[i] != Page_table_entry::create(flags, pa))
								throw Double_insertion();

							Parent::_entries[i] = Page_table_entry::create(flags, pa);
							break;
						}
						default: /* PAGE_TABLE */
						{
							_insert_next_level(i, vo, pa, min(size, end-vo), flags, slab);
						}
					};

					/* check whether we wrap */
					if (end < vo) return;

					size_t sz  = end - vo;
					size = (size > sz) ? size - sz : 0;
					vo += sz;
					pa += sz;
				}
			}

			/**
			 * Remove translations that overlap with a given virtual region
			 *
			 * \param vo    region offset within the tables virtual region
			 * \param size  region size
			 * \param slab  second level page slab allocator
			 */
			void remove_translation(addr_t vo, size_t size, Page_slab * slab)
			{
				if (vo > (vo + size)) throw Invalid_range();

				for (unsigned i; (size > 0) && Parent::_index_by_vo(i, vo);) {

					addr_t end = (vo + Descriptor::VIRT_SIZE)
									 & Descriptor::VIRT_BASE_MASK;

					switch (Descriptor::type(Parent::_entries[i])) {
						case Descriptor::Type::PAGE_TABLE:
						{
							typedef Page_table_descriptor Ptd;
							typedef Next_level_page_table Pt;

							Pt * pt_phys = Ptd::page_table(Parent::_entries[i]);
							Pt * pt      = (Pt *) slab->virt_addr(pt_phys);
							pt = pt ? pt : pt_phys; // TODO hack for core

							addr_t const pt_vo = vo - (vo & Descriptor::VIRT_BASE_MASK);
							pt->remove_translation(pt_vo, min(size, end-vo), slab);

							if (pt->empty()) {
								Descriptor::invalidate(Parent::_entries[i]);
								destroy(slab, pt);
							}
							break;
						}
						default:
						{
							Descriptor::invalidate(Parent::_entries[i]);
						}
					}

					/* check whether we wrap */
					if (end < vo) return;

					size_t sz  = end - vo;
					size = (size > sz) ? size - sz : 0;
					vo += sz;
				}
			}
	};

	class Third_level_table : public Page_table<17, 12>
	{
	} __attribute__((aligned(1<<Third_level_table::ALIGNM_LOG2)));

	class Second_level_table : public Multi_level_page_table<23, 18, Third_level_table>
	{
	} __attribute__((aligned(1<<Second_level_table::ALIGNM_LOG2)));

	class First_level_table : public Multi_level_page_table<31, 24, Second_level_table>
	{
		public:
			typedef Multi_level_page_table<31, 24, Second_level_table> Parent;

			enum {
				MAX_COSTS_PER_TRANSLATION = sizeof(Second_level_table),

				MAX_PAGE_SIZE_LOG2 = 24,
				MIN_PAGE_SIZE_LOG2 = 12,

				MAX_PAGE_SIZE = 1 << MAX_PAGE_SIZE_LOG2,
				MIN_PAGE_SIZE = 1 << MIN_PAGE_SIZE_LOG2,

				MAX_PAGE_OFFSET_MASK = (1 << MAX_PAGE_SIZE_LOG2) - 1,
				MIN_PAGE_OFFSET_MASK = (1 << MIN_PAGE_SIZE_LOG2) - 1,
			};

			void insert_translation(addr_t vo,
											addr_t pa,
											size_t size,
											Page_flags const & flags,
											Page_slab * slab)
			{
				/* sanity check  */
				if ((vo & MIN_PAGE_OFFSET_MASK)
					|| size < MIN_PAGE_SIZE)
					throw Invalid_range();

				Parent::insert_translation(vo, pa, size, flags, slab);
			}

	} __attribute__((aligned(1<<First_level_table::ALIGNM_LOG2)));

	class Context_table
	{
		public:
			enum {
				SIZE_LOG2   = 6,
				SIZE        = 1 << SIZE_LOG2,
				ALIGNM_LOG2 = SIZE_LOG2
			};

			typedef Multi_level_page_table<0, 0, First_level_table>::Page_table_descriptor
				Page_table_descriptor;

			void insert_context(unsigned idx, First_level_table *pt)
			{
				if (idx > MAX_INDEX)
					throw Invalid_range();

				if (Page_table_descriptor::valid(_entries[idx]) &&
					_entries[idx] != Page_table_descriptor::create(pt))
					throw Double_insertion();

				/* compose new descriptor value */
				_entries[idx] = Page_table_descriptor::create(pt);

			}

		protected:
			typename Page_table_descriptor::access_t _entries[SIZE/sizeof(typename Page_table_descriptor::access_t)];

			enum { MAX_INDEX = sizeof(_entries) / sizeof(_entries[0]) - 1 };

	} __attribute__((aligned(1<<Context_table::ALIGNM_LOG2)));
}

namespace Genode { typedef Leon3::First_level_table Translation_table; }

#endif /* _LEON3__TRANSLATION_TABLE_H_ */
