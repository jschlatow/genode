/**
 * \brief  Shadow copy of asm/vdso/processor.h
 * \author Alexander Boettcher
 * \date   2022-03-23
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __ASM_VDSO_PROCESSOR_H
#define __ASM_VDSO_PROCESSOR_H

#ifndef __ASSEMBLY__

#include <linux/delay.h>
#include <linux/jiffies.h>


static __always_inline void rep_nop(void)
{
	asm volatile("rep; nop" ::: "memory");
}


extern void intel_emul_cpu_relax(void);

static __always_inline void cpu_relax(void)
{
	/*
	 * break busy loop of slchi() in drivers/i2c/algos/i2c-algo-bit.c
	 * without re-scheduling another task, which breaks execution
	 * assumptions, e.g. drivers/gpu/drm/i915/intel_uncore.c
	 * spin_lock_irq()+__intel_wait_for_register_fw() combination
	 */
	intel_emul_cpu_relax();
}

#endif /* __ASSEMBLY__ */

#endif /* __ASM_VDSO_PROCESSOR_H */
