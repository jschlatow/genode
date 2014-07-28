/*
 * \brief  Serial output driver specific for the LEON3
 * \author Johannes Schlatow
 * \date   2014-07-24
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__LEON3__DRIVERS__SERIAL_LOG_H_
#define _INCLUDE__LEON3__DRIVERS__SERIAL_LOG_H_

/* Genode includes */
#include <board.h>
#include <drivers/uart/leon3_uart_base.h>

namespace Genode
{
	/**
	 * Serial output driver specific for the LEON3 APBUART
	 */
	class Serial_log : public Leon3_uart_base
	{
		public:

			/**
			 * Constructor
			 *
			 * \param baud_rate  targeted transfer baud-rate
			 */
			Serial_log(unsigned const baud_rate) :
				Leon3_uart_base(Board::APBUART_MMIO_BASE)
			{ }
	};
}

#endif /* _INCLUDE__LEON3__DRIVERS__SERIAL_LOG_H_ */

