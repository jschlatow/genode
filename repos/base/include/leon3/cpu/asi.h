/*
 * \brief  Accessing alternative address spaces on LEON3 using ASI
 * \author Johannes Schlatow
 * \date   2014-07-28
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__LEON3__CPU__ASI_H_
#define _INCLUDE__LEON3__CPU__ASI_H_

#include <util/register.h>
#include <base/stdint.h>

namespace Leon3 {
	using namespace Genode;

	template <unsigned ASI>
	inline
	umword_t
	read_alternative(addr_t addr)
	{
		umword_t ret;
		__asm__ __volatile__(
			"lda [%1] %2, %0"
				  : "=r" (ret)
				  : "r" (addr),
				    "i" (ASI)
		);
		return ret;
	}

	template <unsigned ASI>
	inline
	void
	write_alternative(addr_t addr, umword_t value)
	{
		umword_t ret;
		__asm__ __volatile__(
			"sta %0, [%1] %2"
				  :
				  : "r"(value),
				    "r"(addr),
				    "i"(ASI)
		);
	}

	struct Asi
	{
		enum {
			Flush_caches  = 0x10,
			Flush_tlb     = 0x18,
			Mmu_registers = 0x19,
			Mmu_bypass    = 0x1C
		};

		template <unsigned ASI, unsigned ADDR>
		struct Register : Genode::Register<32>
		{
			static access_t read()
			{
				return read_alternative<ASI>(ADDR);
			}

			static void write(access_t const v)
			{
				write_alternative<ASI>(ADDR, v);
			}
		};
	};
}

#endif /* _INCLUDE__LEON3__CPU__ASI_H_ */
