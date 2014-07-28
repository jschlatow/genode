/*
 * \brief  Board definitions for the LEON3
 * \author Johannes Schlatow
 * \date   2014-07-24
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__PLATFORM__BOARD_BASE_H_
#define _INCLUDE__PLATFORM__BOARD_BASE_H_

/* Genode includes */
#include <util/mmio.h>

namespace Genode
{
	struct Board_base
	{
		enum {
			RAM_0_BASE = 0x40000000, /* FIXME get value from PNP section */
			RAM_0_SIZE = 0x10000000, /* FIXME get value from PNP section */

//			MMIO_0_BASE = 0x20000000,
//			MMIO_0_SIZE = 0x02000000,

//			LEON3_NUM_DEVICE_INFO = 64,

			APBUART_MMIO_BASE           = 0x80000100,

			GPTIMER_MMIO_BASE           = 0x80000300, /* FIXME get real value from PNP section */
			GPTIMER_PRESCALE_CLOCK      = 1 * 1000 * 1000,  /*  1 MHz */

			SYSTEM_CLOCK                = 40 * 1000 * 1000, /* 40 MHz */

			IRQMP_MMIO_BASE             = 0x80000200, /* FIXME get real value from PNP section */

//			LEON3_VENDOR_GAISLER        = 0x1,
//			LEON3_VENDOR_ESA            = 0x4,
//			LEON3_DEVICEID_MCTRL        = 0xF,
//			LEON3_DEVICEID_XILMIG       = 0x6B,
//
//			LEON3_AHB_BAR_MASK_SHIFT    = 4,
//			LEON3_AHB_BAR_MASK_MASK     = 0xFFF,
//			LEON3_AHB_BAR_ADDR_SHIFT    = 20,
//			LEON3_AHB_BAR_ADDR_MASK     = 0xFFF,
//
//			LEON3_MEMCFG2               = 0x80000004,
//			LEON3_MEMCFG2_SDRAMSZ_SHIFT =  23,
//			LEON3_MEMCFG2_SDRAMSZ_MASK  =   7,
//			LEON3_MEMCFG2_RAMSZ_SHIFT   =   9,
//			LEON3_MEMCFG2_SRAM_DISABLEF =  13,
//			LEON3_MEMCFG2_RAMSZ_MASK    = 0xF,
//
//			AHB_MASTER_TABLE      = 0xFFFFF000,
//			AHB_SLAVE_TABLE       = 0xFFFFF800,


			SECURITY_EXTENSION = 0,

			/* CPU cache */
			CACHE_LINE_SIZE_LOG2 = 2, /* FIXME get correct value from board spec */
		};
	};
}

#endif /* _INCLUDE__PLATFORM__BOARD_BASE_H_ */

