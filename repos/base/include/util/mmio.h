/*
 * \brief  Type-safe, fine-grained access to a continuous MMIO region
 * \author Martin stein
 * \date   2011-10-26
 */

/*
 * Copyright (C) 2011-2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__UTIL__MMIO_H_
#define _INCLUDE__UTIL__MMIO_H_

/* Genode includes */
#include <util/io.h>

namespace Genode {

	class Mmio_base;
	class Mmio;
}

/**
 * Raw IO access implementation for MMIO
 */
class Genode::Mmio_base
{
	friend Io_base;

	private:

		addr_t const _base;

		/**
		 * Write '_ACCESS_T' typed 'value' to MMIO base + 'o'
		 */
		template <typename ACCESS_T>
		inline void _write(off_t const offset, ACCESS_T const value)
		{
			addr_t const dst = _base + offset;
			*(ACCESS_T volatile *)dst = value;
		}

		/**
		 * Read '_ACCESS_T' typed from MMIO base + 'o'
		 */
		template <typename ACCESS_T>
		inline ACCESS_T _read(off_t const &offset) const
		{
			addr_t const dst = _base + offset;
			ACCESS_T const value = *(ACCESS_T volatile *)dst;
			return value;
		}

	public:

		/**
		 * Constructor
		 *
		 * \param base  base address of targeted MMIO region
		 */
		Mmio_base(addr_t const base) : _base(base) { }
};


/**
 * Type-safe, fine-grained access to a continuous MMIO region
 *
 * For further details refer to the documentation of the 'Io' class in
 * 'util/io.h'
 */
struct Genode::Mmio : Mmio_base, Io<Mmio_base>
{
	/**
	 * Constructor
	 *
	 * \param base  base address of targeted MMIO region
	 */
	Mmio(addr_t const base) : Mmio_base(base),
	                          Io(*static_cast<Mmio_base *>(this)) { }
};

#endif /* _INCLUDE__UTIL__MMIO_H_ */
