/*
 * \brief  Base driver for Parallella Board
 * \author Johannes Schlatow
 * \date   2015-06-30
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__PARALLELLA__DRIVERS__BOARD_BASE_H_
#define _INCLUDE__PARALLELLA__DRIVERS__BOARD_BASE_H_

#include <zynq/drivers/board_base.h>

namespace Genode { struct Board_base; }

/**
 * Base driver for the Parallella platform
 */
struct Genode::Board_base : Zynq::Board_base
{
	enum
	{
		/* clocks (assuming 6:2:1 mode) */
		CPU_1X_CLOCK   = 133000000,
		CPU_6X4X_CLOCK = 6*CPU_1X_CLOCK,

		CORTEX_A9_CLOCK             = CPU_6X4X_CLOCK,
		CORTEX_A9_PRIVATE_TIMER_CLK = CORTEX_A9_CLOCK,
	};
};

#endif /* _INCLUDE__PARALLELLA__DRIVERS__BOARD_BASE_H_ */
