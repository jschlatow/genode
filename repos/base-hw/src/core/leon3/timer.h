/*
 * \brief  LEON3 timer for kernel
 * \author Johannes Schlatow
 * \date   2014-07-24
 *
 * TODO check: do we need to use a {read,write}_alternative<> in order to
 *             access a different address space?
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _LEON3__TIMER_H_
#define _LEON3__TIMER_H_

/* Genode includes */
#include <util/mmio.h>
#include <base/stdint.h>
#include <base/printf.h>
#include <drivers/board_base.h>

namespace Kernel { class Timer; }


class Kernel::Timer : public Genode::Mmio
{
	/* scaler value register */
	struct Sc : Register<0x0, 32>
	{
		struct Value : Bitfield<0, 16> { };
	};

	/* scaler reload value register */
	struct Scr : Register<0x04, 32>
	{
		struct Value : Bitfield<0, 16> { };
	};

	/* configuration register */
	struct Cfg : Register<0x08, 32>
	{
		struct Timers : Bitfield<0, 3> { }; /* number of timers (read-only) */
		struct Irq    : Bitfield<3, 5> { }; /* timer IRQ (read-only) */
		struct Si     : Bitfield<8, 1> { }; /* separate interrupts (Irq+n) */
		struct Fr_dis : Bitfield<9, 1> { }; /* disable timer freeze */
	};

	/* Timer 1 counter value register */
	struct Tmr1_cnt : Register<0x10, 32>
	{
		struct Value : Bitfield<0, 32> { };
	};

	/* Timer 1 reload value register */
	struct Tmr1_rel : Register<0x14, 32>
	{
		struct Value : Bitfield<0, 32> { };
	};

	/* Timer 1 control register */
	struct Tmr1_ctrl : Register<0x18, 32>
	{
		struct En     : Bitfield<0, 1> { }; /* enable timer */
		struct Res    : Bitfield<1, 1> { }; /* restart timer on underflow */
		struct Ld     : Bitfield<2, 1> { }; /* load timer with reload value */
		struct Int_en : Bitfield<3, 1> { }; /* enable underflow interrupt */
		struct Int    : Bitfield<4, 1> { }; /* interrupt pending (cleared by writing '1') */
		struct Ch_en  : Bitfield<5, 1> { }; /* chain timer with preceeding timer */
		struct Dh     : Bitfield<6, 1> { }; /* debug halt (read-only) */
	};

	private:

		typedef Genode::uint32_t   uint32_t;
		typedef Genode::Board_base Board_base;

		uint32_t _num_timers;		

	public:

		Timer(bool init=true) : Mmio(Board_base::GPTIMER_MMIO_BASE)
		{
			_num_timers = read<Cfg::Timers>();

			if (init) {
				/* configure prescaler */
				uint32_t prescale = Board_base::SYSTEM_CLOCK / Board_base::GPTIMER_PRESCALE_CLOCK;

				/* we only have a 16-bit prescaler */
				assert(prescale <= (1 << 16));

				write<Scr>(prescale);

				/* FIXME use separate interrupts */
				write<Cfg::Si>(0);

				/* run timer #1 in one-shot mode */
				write<Tmr1_ctrl::Res>(0);

				/* enable IRQ for timer #1 */
				write<Tmr1_ctrl::Int_en>(1);

				/* enable timer #1 */
				write<Tmr1_ctrl::En>(1);
			}
		}

		static unsigned interrupt_id(unsigned)
		{
			/* FIXME support multiple timer IRQs */
			Timer timer(false);
			return timer.read<Cfg::Irq>();
		}

		inline void start_one_shot(uint32_t const tics, unsigned)
		{
			/* set reload value */
			write<Tmr1_rel>(tics);
			/* clear interrupt */
			write<Tmr1_ctrl::Int>(1);
			/* load reload value into timer */
			write<Tmr1_ctrl::Ld>(1);
		}

		static uint32_t ms_to_tics(unsigned const ms)
		{
			return (Board_base::GPTIMER_PRESCALE_CLOCK / 1000) * ms;
		}

		void clear_interrupt(unsigned)
		{
			write<Tmr1_ctrl::Int>(1);
		}
};

#endif /* _LEON3__TIMER_H_ */

