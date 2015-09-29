/*
 * \brief  Base driver for Zedboard
 * \author Mark Albers
 * \date   2015-09-29
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
		PS_CLOCK = 33333333,
		ARM_PLL_CLOCK = 1333333*1000,
		DDR_PLL_CLOCK = 1066667*1000,
		IO_PLL_CLOCK  = 1000*1000*1000,
		CPU_1x_CLOCK = 111111115,
		CPU_1X_CLOCK = 111111115,
		CPU_6x4x_CLOCK = 6*CPU_1x_CLOCK,
		CPU_3x2x_CLOCK = 3*CPU_1x_CLOCK,
		CPU_2x_CLOCK   = 2*CPU_1x_CLOCK,

		CORTEX_A9_CLOCK             = CPU_6x4x_CLOCK,
		CORTEX_A9_PRIVATE_TIMER_CLK = CORTEX_A9_CLOCK,

		UART_MMIO_BASE = UART_1_MMIO_BASE,

		RAM_0_SIZE = 0x20000000, /* 512MB */
		DDR_CLOCK  = 533333313,

		FCLK_CLK0  = 100*1000*1000, /* AXI */
		FCLK_CLK1  = 20250*1000, /* Cam */
		FCLK_CLK2  = 150*1000*1000, /* AXI HP */

		I2C0_MMIO_BASE = 0xE0004000,
		I2C1_MMIO_BASE = 0xE0005000,
		I2C_MMIO_SIZE = 0x1000,

		UART_0_IRQ = 59,
		UART_1_IRQ = 82,

		QSPI_CLOCK = 200*1000*1000,
		ETH_CLOCK  = 125*1000*1000,
		SD_CLOCK   =  50*1000*1000,

		/* GPIO */
		GPIO_MMIO_SIZE = 0x1000,

		/* VDMA */
		VDMA_MMIO_SIZE = 0x10000,

		/* TTC (triple timer counter) */
		TTC0_MMIO_BASE = MMIO_1_BASE + 0x1000,
		TTC0_MMIO_SIZE = 0xfff,
		TTC1_MMIO_BASE = MMIO_1_BASE + 0x2000,
		TTC1_MMIO_SIZE = 0xfff,
		TTC0_IRQ_0     = 42,
		TTC0_IRQ_1     = 43,
		TTC0_IRQ_2     = 44,
		TTC1_IRQ_0     = 69,
		TTC1_IRQ_1     = 70,
		TTC1_IRQ_2     = 71,
	};
};

#endif /* _INCLUDE__ZEDBOARD__DRIVERS__BOARD_BASE_H_ */
