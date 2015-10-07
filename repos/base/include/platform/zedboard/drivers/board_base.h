/*
 * \brief  Base driver for Zedboard Board
 * \author Johannes Schlatow
 * \date   2015-09-04
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__ZEDBOARD__DRIVERS__BOARD_BASE_H_
#define _INCLUDE__ZEDBOARD__DRIVERS__BOARD_BASE_H_

#include <zynq/drivers/board_base.h>

namespace Genode { struct Board_base; }

/**
 * Base driver for the Zedboard platform
 */
struct Genode::Board_base : Zynq::Board_base
{
	enum
	{
		/* clocks (assuming 6:2:1 mode) */
		CPU_1X_CLOCK   = 133000000,      /* TODO verify */
		CPU_6X4X_CLOCK = 6*CPU_1X_CLOCK,

		CORTEX_A9_CLOCK             = CPU_6X4X_CLOCK,
		CORTEX_A9_PRIVATE_TIMER_CLK = CORTEX_A9_CLOCK,

		RAM_0_SIZE = 0x20000000, /* 512MB */
		DDR_CLOCK  = 533333313,
		FCLK_CLK0  = 100*1000*1000, /* AXI */
		FCLK_CLK1  = 20250*1000, /* Cam */
		FCLK_CLK2  = 150*1000*1000, /* AXI HP */
	};
};

#endif /* _INCLUDE__ZEDBOARD__DRIVERS__BOARD_BASE_H_ */
