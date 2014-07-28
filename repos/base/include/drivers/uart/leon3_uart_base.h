/*
 * \brief  Driver base for LEON3 APB UART
 * \author Johannes Schlatow
 * \date   2014-07-24
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__DRIVERS__UART__LEON3_UART_BASE_H_
#define _INCLUDE__DRIVERS__UART__LEON3_UART_BASE_H_

/* Genode includes */
#include <util/mmio.h>

namespace Genode
{
	/**
	 * Driver base for LEON3 APB UART
	 */
	class Leon3_uart_base : Mmio
	{
		/**
		 * Data register
		 */
		struct Dr : Register<0x00, 32>
		{
			struct Data : Bitfield<0, 8> { }; /* receive/transmit data */
		};

		/**
		 * Control register
		 */
		struct Cr : Register<0x08, 32>
		{
			struct Rx_en  : Bitfield<0, 1> { }; /* enable receiver */
			struct Tx_en  : Bitfield<1, 1> { }; /* enable transmitter */
			struct Rxi_en : Bitfield<2, 1> { }; /* enable RX interrupt */
			struct Txi_en : Bitfield<3, 1> { }; /* enable TX interrupt */

			struct Ps     : Bitfield<4, 1>      /* select parity */
			{
				enum {
					_EVEN = 0,
					_ODD  = 1
				};
			};

			struct Pr_en  : Bitfield<5, 1>  { }; /* enable parity */
			struct Fl_en  : Bitfield<6, 1>  { }; /* enable CTS/RTS flow control */
			struct Lb_en  : Bitfield<7, 1>  { }; /* enable loopback */
			struct Ec_en  : Bitfield<8, 1>  { }; /* enable external clock */
			struct Tf_en  : Bitfield<9, 1>  { }; /* enable transmitter FIFO interrupt */
			struct Rf_en  : Bitfield<10, 1> { }; /* enable receiver FIFO interrupt */
			struct Db_en  : Bitfield<11, 1> { }; /* enable FIFO debug mode */
			struct Bi_en  : Bitfield<12, 1> { }; /* enable break interrupt */
			struct Di_en  : Bitfield<13, 1> { }; /* enable delayed interrupt */
			struct Si_en  : Bitfield<14, 1> { }; /* enable transmitter shift register empty interrupt */
			struct Fa     : Bitfield<31, 1> { }; /* set whether FIFOs are available */

		};

		/**
		 * Status register
		 */
		struct Sr : Register<0x04, 32>
		{
			struct Dr : Bitfield<0, 1> { }; /* data ready bit */
			struct Ts : Bitfield<1, 1> { }; /* transmit shift empty */
			struct Te : Bitfield<2, 1> { }; /* transmit fifo empty */
			struct Br : Bitfield<3, 1> { }; /* BREAK received */
			struct Ov : Bitfield<4, 1> { }; /* overrun */
			struct Pe : Bitfield<5, 1> { }; /* parity error */
			struct Fe : Bitfield<6, 1> { }; /* framing error */
			struct Th : Bitfield<7, 1> { }; /* transmitter FIFO half full */
			struct Rh : Bitfield<8, 1> { }; /* receiver FIFO half full */
			struct Tf : Bitfield<9, 1> { }; /* transmitter FIFO full */
			struct Rf : Bitfield<10,1> { }; /* receiver FIFO full */
			struct Tcnt : Bitfield<20,5> { }; /* receiver FIFO count */
			struct Rcnt : Bitfield<26,5> { }; /* receiver FIFO count */
		};

		/**
		 * Scaler register
		 */
		struct Scr : Register<0x0C, 32>
		{
			enum { SBITS = 12 };
			struct R_val : Bitfield<0, SBITS> { }; /* scaler reload value */
		};


		/**
		 * Transmit character 'c' without care about its type
		 */
		inline void _put_char(char const c)
		{
			while (read<Sr::Tf>()) ;
			write<Dr::Data>(c);
		}

		public:

			/**
			 * Constructor
			 *
			 * \param base  device MMIO base
			 */
			explicit Leon3_uart_base(addr_t const base) : Mmio(base)
			{
				write<Cr>(read<Cr>() | Cr::Tx_en::bits(1));
			}

			/**
			 * Print character 'c' through the UART
			 */
			inline void put_char(char const c)
			{
				enum { ASCII_LINE_FEED       = 10,
				       ASCII_CARRIAGE_RETURN = 13 };

				/* prepend line feed with carriage return */
				if (c == ASCII_LINE_FEED) _put_char(ASCII_CARRIAGE_RETURN);

				/* transmit character */
				_put_char(c);
			}
	};
}

#endif /* _INCLUDE__DRIVERS__UART__IMX31_UART_BASE_H_ */

