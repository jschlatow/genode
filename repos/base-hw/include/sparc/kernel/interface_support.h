/*
 * \brief  Interface between kernel and userland
 * \author Johannes Schlatow
 * \date   2014-07-24
 *
 * TODO implement kernel interface for sparc
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _KERNEL__INTERFACE_SUPPORT_H_
#define _KERNEL__INTERFACE_SUPPORT_H_

/* Genode includes */
#include <base/stdint.h>

namespace Kernel
{
	typedef Genode::uint32_t Call_arg;
	typedef Genode::uint32_t Call_ret;

	/**
	 * Registers that are provided by a kernel thread-object for user access
	 */
	struct Thread_reg_id
	{
		enum {
			L0            = 0,
			L1            = 1,
			L2            = 2,
			L3            = 3,
			L4            = 4,
			L5            = 5,
			L6            = 6,
			L7            = 7,
			I0            = 8,
			I1            = 9,
			I2            = 10,
			I3            = 11,
			I4            = 12,
			I5            = 13,
			I6            = 14,
			I7            = 15,
			SP            = 13,
			IP            = 15,
			CPSR          = 16,
			CPU_EXCEPTION = 17,
			FAULT_TLB     = 18,
			FAULT_ADDR    = 19,
			FAULT_WRITES  = 20,
			FAULT_SIGNAL  = 21,
		};
	};

	/**
	 * Events that are provided by a kernel thread-object for user handling
	 */
	struct Thread_event_id
	{
		enum { FAULT = 0 };
	};
}

#endif /* _KERNEL__INTERFACE_SUPPORT_H_ */

