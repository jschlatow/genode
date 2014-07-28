/*
 * \brief  Atomic operations for LEON3
 * \author Johannes Schlatow
 * \date   2014-07-24
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__LEON3__CPU__ATOMIC_H_
#define _INCLUDE__LEON3__CPU__ATOMIC_H_

namespace Genode {

	/**
	 * Atomic compare and exchange
	 *
	 * This function compares the value at dest with cmp_val.
	 * If both values are equal, dest is set to new_val. If
	 * both values are different, the value at dest remains
	 * unchanged.
	 *
	 * \return  1 if the value was successfully changed to new_val,
	 *          0 if cmp_val and the value at dest differ.
	 */
	inline int cmpxchg(volatile int *dest, int cmp_val, int new_val)
	{
		/*
		 * LEON3 borrows the Compare and Swap operation from sparc v9
		 * if (*dest == cmp_val) {
		 * 	*dest = new_val;
		 * 	new_val = cmp_val;
		 * }
		 * else {
		 *    new_val = *dest;
		 * }
		 */
		__asm__ __volatile__(
			"  cas [%[dest]], %[cmp_val], %[new_val]\n"
			: [new_val] "+r" (new_val)
			: [dest] "r" (dest), [cmp_val] "r" (cmp_val)
			: "memory");
		return (new_val == cmp_val);
	}
}

#endif /* _INCLUDE__LEON3__CPU__ATOMIC_H_ */
