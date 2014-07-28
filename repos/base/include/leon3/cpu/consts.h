/*
 * \brief  Constants definitions for the LEON3 architecture.
 * \author Johannes Schlatow
 * \date   2014-07-24
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__LEON3__CPU__CONSTS_H_
#define _INCLUDE__LEON3__CPU__CONSTS_H_

#include <base/stdint.h>

namespace Abi {

	/**
	 * TODO
	 * On ARM a call (or branch) will not change the stack pointer, so we do not
	 * need stack adjustment
	 */
	static constexpr Genode::size_t stack_adjustment() { return 0; }

	/**
	 * TODO
	 * Do ABI specific initialization to a freshly created stack
	 */
	inline void init_stack(Genode::addr_t) { }
}

#endif /* _INCLUDE__LEON3__CPU__CONSTS_H_ */
