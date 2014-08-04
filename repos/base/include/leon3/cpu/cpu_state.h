/*
 * \brief  CPU state on LEON3
 * \author Johannes Schlatow
 * \date   2014-07-28
 *
 * TODO implement CPU state for LEON3
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__LEON3__CPU__CPU_STATE_H_
#define _INCLUDE__LEON3__CPU__CPU_STATE_H_

/* Genode includes */
#include <base/stdint.h>

namespace Genode {

	/**
	 * Basic CPU state
	 */
	struct Cpu_state
	{
		/**
		 * Native exception types
		 */
		enum Cpu_exception {
			RESET                  = 0,
		};

		/**
		 * Registers
		 */
		addr_t g0, g1, g2, g3, g4, g5, g6, g7; /* global registers */
		addr_t i0, i1, i2, i3, i4, i5, i6, i7; /* input registers */
		addr_t l0, l1, l2, l3, l4, l5, l6, l7; /* local registers */
		addr_t o0, o1, o2, o3, o4, o5;    /* output registers o0-o5 */
		union {
			addr_t sp;                        /* stack pointer (o6) */
			addr_t o6;
		};
		addr_t o7;
		addr_t ip;                        /* instruction pointer */
		addr_t cpsr;                      /* current program status register */
		addr_t cpu_exception;             /* last trap code */
//		addr_t user_stack[];                /*  */
	};

	/**
	 * Dummy implementation for base-hw
	 */
	struct Cpu_state_modes : Cpu_state
	{
	};
}

#endif /* _INCLUDE__LEON3__CPU__CPU_STATE_H_ */
