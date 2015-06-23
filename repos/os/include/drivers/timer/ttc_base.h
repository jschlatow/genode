/*
 * \brief  Basic driver for the Zynq Triple Counter Timer
 * \author Johannes Schlatow
 * \author Martin Stein
 * \date   2015-03-05
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__DRIVERS__TIMER__TTC_BASE_H_
#define _INCLUDE__DRIVERS__TIMER__TTC_BASE_H_

/* Genode includes */
#include <util/mmio.h>
#include <drivers/board_base.h>

namespace Genode { class Ttc_base; }

/**
 * Basic driver for the Zynq TTC
 *
 * Uses the internal timer 0 of the TTC. For more details, see Xilinx ug585.
 */
class Genode::Ttc_base : public Mmio
{
	private:

		enum { TICS_PER_US = Genode::Board_base::CPU_1X_CLOCK / 1000 / 1000 };

		/**
		 * Counter control register
		 */
		struct Control : Register<0x0c, 8>
		{
			struct Disable    : Bitfield<0, 1> { };
			struct Mode       : Bitfield<1, 1> { enum { INTERVAL = 1 }; };
			struct Decrement  : Bitfield<2, 1> { };
			struct Wave_en    : Bitfield<5, 1> { };
		};

		/**
		 * Counter value
		 */
		struct Value    : Register<0x18, 16> { };
		struct Interval : Register<0x24, 16> { };
		struct Match1   : Register<0x30, 16> { };
		struct Match2   : Register<0x3c, 16> { };
		struct Match3   : Register<0x48, 16> { };
		struct Irq      : Register<0x54,  8> { };
		struct Irqen    : Register<0x60,  8> { };

		void _disable()
		{
			write<Control::Disable>(0);
			read<Irq>();
		}

	public:

		/**
		 * Constructor
		 */
		Ttc_base(addr_t const mmio_base) : Mmio(mmio_base)
		{
			_disable();

			/* enable all interrupts */
			write<Irqen>(~0);

			/* set match registers to 0 */
			write<Match1>(0);
			write<Match2>(0);
			write<Match3>(0);
		}

		/**
		 * Count down 'value', raise IRQ output, wrap counter and continue
		 */
		void run_and_wrap(unsigned long const tics)
		{
			_disable();

			/* configure timer for a one-shot */
			Control::access_t control = 0;
			Control::Mode::set(control, Control::Mode::INTERVAL);
			Control::Decrement::set(control, 1);
			Control::Wave_en::set(control, 1);
			write<Control>(control);

			/* load and enable timer */
			write<Interval>(tics);
			write<Control::Disable>(0);
		}

		/**
		 * Get timer value and corresponding wrapped status of timer
		 */
		unsigned long value(bool & wrapped) const
		{
			unsigned long v = read<Value>();
			wrapped = (bool)read<Irq>();
			return wrapped ? read<Value>() : v;
		}

		/**
		 * Translate native timer value to microseconds
		 */
		static unsigned long tics_to_us(unsigned long const tics) {
			return tics / TICS_PER_US; }

		/**
		 * Translate microseconds to a native timer value
		 */
		static unsigned long us_to_tics(unsigned long const us) {
			return us * TICS_PER_US; }

		/**
		 * Translate native timer value to microseconds
		 */
		unsigned long max_value() const { return (Interval::access_t)~0; }
};

#endif /* _INCLUDE__DRIVERS__TIMER__A9_PRIVATE_TIMER_H_ */

