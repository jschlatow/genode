/*
 * \brief  Implementation of LEON3's SRMMU 
 * \author Johannes Schlatow
 * \date   2014-07-28
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _LEON3__MMU_H_
#define _LEON3__MMU_H_

/* Genode includes */
#include <util/register.h>
#include <cpu/asi.h>

/* core includes */
#include <board.h>

namespace Leon3
{
	using namespace Genode;

	struct Mmu
	{
		/* control register */
		struct Ctrl : Asi::Register<Asi::Mmu_registers, 0x000>
		{
			struct En  : Bitfield<0, 1>  { }; /* enable MMU */
			struct Nf  : Bitfield<1, 1>  { }; /* no fault */
			struct St  : Bitfield<1, 14> { }; /* separate TLBs (read-only) */
			struct Td  : Bitfield<1, 15> { }; /* disable TLB */
			struct Psz : Bitfield<2, 16>      /* minimum page size (might by read-only) */
			{
				enum {
					K4  = 0x0,
					K8  = 0x1,
					K16 = 0x2,
					K32 = 0x3,
				};
			};

			struct Dtlb : Bitfield<3, 18> { }; /* log2 of number of DTLB entries */
			struct Itlb : Bitfield<3, 21> { }; /* log2 of number of ITLB entries */
		};

		/* context table pointer register */
		struct Ctp : Asi::Register<Asi::Mmu_registers, 0x100>
		{
			enum {
				CTP_SHIFT = 4,
			};

			static inline
			access_t
			read()
			{
				return read() << CTP_SHIFT;
			}

			static inline
			void
			write(access_t const v)
			{
				write(v >> CTP_SHIFT);
			}
		};

		/* context register */
		struct Ctx : Asi::Register<Asi::Mmu_registers, 0x200>
		{
			struct Value : Bitfield<0, 4> { };  /* we have 16 contexts */
		};

		/* fault status register */
		struct Fsr : Asi::Register<Asi::Mmu_registers, 0x300>
		{
			struct Ow    : Bitfield<0, 1> { }; /* overwrite bit */
			struct Fav   : Bitfield<1, 1> { }; /* fault address valid */
			struct Ft    : Bitfield<2, 3>      /* fault type */
			{
				enum {
					None        = 0x0,
					Invalid     = 0x1,
					Protected   = 0x2,
					Privilege   = 0x3,
					Translation = 0x4,
					Access_bus  = 0x5,
					Internal    = 0x6,
					Reserved    = 0x7,
				};
			};

			struct At_0 : Bitfield<3, 1>       /* access type 0 */
			{
				enum {
					User = 0x0,
					Supervisor = 0x1,
				};
			};

			struct At_1 : Bitfield<4, 1>       /* access type 1 */
			{
				enum {
					Data  = 0x0,
					Instr = 0x1,
				};
			};

			struct At_2 : Bitfield<5, 1>       /* access type 2 */
			{
				enum {
					Load  = 0x0,
					Store = 0x1,
				};
			};

			struct At   : Bitset_3<At_0, At_1, At_2> { }; /* access type */

			struct Lvl  : Bitfield<8, 2> { };  /* faulting page table level  */
			struct Ebe  : Bitfield<10,8> { };  /* external bus error */
		};

		/* fault address register */
		struct Far : Asi::Register<Asi::Mmu_registers, 0x400>
		{ };
	};
}

#endif /* _LEON3__MMU_H_ */

