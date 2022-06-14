/*
 * \brief  Interface between kernel and hypervisor
 * \author Stefan Kalkowski
 * \date   2022-06-13
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SPEC__ARM_V8__VIRTUALIZATION_HYPERVISOR_H_
#define _SPEC__ARM_V8__VIRTUALIZATION_HYPERVISOR_H_

#include <base/stdint.h>

namespace Hypervisor {

	enum Call_number {
		WORLD_SWITCH   = 0,
		TLB_INVALIDATE = 1,
	};

	using Call_arg = Genode::umword_t;
	using Call_ret = Genode::umword_t;

	extern "C"
	Call_ret hypervisor_call(Call_arg call_id,
	                         Call_arg arg0,
	                         Call_arg arg1,
	                         Call_arg arg2,
	                         Call_arg arg3);


	inline void invalidate_tlb(Call_arg ttbr)
	{
		hypervisor_call(TLB_INVALIDATE, ttbr, 0, 0, 0);
	}


	inline void switch_world(Call_arg guest_state,
	                         Call_arg host_state,
	                         Call_arg pic_state,
	                         Call_arg ttbr)
	{
		hypervisor_call(WORLD_SWITCH, guest_state, host_state, pic_state, ttbr);
	}
}

#endif /* _SPEC__ARM_V8__VIRTUALIZATION_HYPERVISOR_H_ */
