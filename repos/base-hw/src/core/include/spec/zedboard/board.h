/*
 * \brief  Board driver for core on Zynq/Zedboard
 * \author Mark Albers
 * \date   2015-03-12
 * \author Johannes Schlatow
 * \date   2014-12-15
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _BOARD_H_
#define _BOARD_H_

/* core includes */
#include <spec/zynq/board_support.h>

namespace Genode
{
	class Board : public Zynq::Board
	{
		public:
			enum {
				UART_MMIO_BASE = UART_1_MMIO_BASE,
				RAM_0_SIZE = 0x20000000, /* 512MB */
				DDR_CLOCK  = 533333313,
				FCLK_CLK0  = 100*1000*1000, /* AXI */
				FCLK_CLK1  = 20250*1000, /* Cam */
				FCLK_CLK2  = 150*1000*1000, /* AXI HP */
			};
	};
}

#endif /* _BOARD_H_ */
