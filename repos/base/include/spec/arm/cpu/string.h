/*
 * \brief  ARM-specific memcpy
 * \author Sebastian Sumpf
 * \author Stefan Kalkowski
 * \date   2012-08-02
 */

/*
 * Copyright (C) 2012-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__SPEC__ARM__CPU__STRING_H_
#define _INCLUDE__SPEC__ARM__CPU__STRING_H_

namespace Genode {

	/**
	 * Copy memory block
	 *
	 * \param dst   destination memory block
	 * \param src   source memory block
	 * \param size  number of bytes to copy
	 *
	 * \return      Number of bytes not copied
	 */
	inline size_t memcpy_cpu(void *dst, const void *src, size_t size)
	{
		unsigned char *d = (unsigned char *)dst, *s = (unsigned char *)src;

		/* fetch the first cache line */
		asm volatile ("pld [%0, #0]\n\t" : "+r" (s));

		/* check 32-byte alignment */
//		size_t d_align = (size_t)d & 0x1f;
//		size_t s_align = (size_t)s & 0x1f;
		size_t d_align = (size_t)d & 0x1f;
		size_t s_align = (size_t)s & 0x1f;

		/* only same word-alignments work for the following LDM/STM loop */
		if ((d_align & 0x3) != (s_align & 0x3))
			return size;

		/* copy to 32-byte alignment */
//		for (; (size > 0) && (s_align > 0) && (s_align < 32);
		for (; (size > 0) && (s_align > 0) && (s_align < 32);
		     s_align++, *d++ = *s++, size--);

		/**
		 * On Cortex-A9 (Zynq), starting the loop from a 24-byte offset seems to
		 * gain a few more MiB/s (1051 vs 1068). We keep it cache-line aligned
		 * though until this is validated on other SoCs.
		 */

		/* copy 32 byte chunks */
		for (; size >= 32; size -= 32) {
			asm volatile ("pld [%0, #160]\n\t"
			              "ldmia %0!, {r3 - r10} \n\t"
			              "stmia %1!, {r3 - r10} \n\t"
			              : "+r" (s), "+r" (d)
			              :: "r3","r4","r5","r6","r7","r8","r9","r10");
		}
		return size;
	}
}

#endif /* _INCLUDE__SPEC__ARM__CPU__STRING_H_ */
