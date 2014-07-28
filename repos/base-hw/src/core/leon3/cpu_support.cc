/*
 * \brief   CPU specific implementations of core
 * \author  Martin Stein
 * \author Stefan Kalkowski
 * \date    2013-11-11
 */

/*
 * Copyright (C) 2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* core includes */
#include <kernel/thread.h>
#include <kernel/pd.h>
#include <kernel/vm.h>

using namespace Kernel;


/********************************
 ** Kernel::Thread_cpu_support **
 ********************************/

Thread_cpu_support::Thread_cpu_support(Thread * const t)
:
	_fault(t),
	_fault_pd(0),
	_fault_addr(0),
	_fault_writes(0),
	_fault_signal(0)
{ }


/********************
 ** Kernel::Thread **
 ********************/

addr_t Thread::* Thread::_reg(addr_t const id) const
{
	static addr_t Thread::* const _regs[] = {
		/* [0]  */ (addr_t Thread::*)&Thread::l0,
		/* [1]  */ (addr_t Thread::*)&Thread::l1,
		/* [2]  */ (addr_t Thread::*)&Thread::l2,
		/* [3]  */ (addr_t Thread::*)&Thread::l3,
		/* [4]  */ (addr_t Thread::*)&Thread::l4,
		/* [5]  */ (addr_t Thread::*)&Thread::l5,
		/* [6]  */ (addr_t Thread::*)&Thread::l6,
		/* [7]  */ (addr_t Thread::*)&Thread::l7,
		/* [8]  */ (addr_t Thread::*)&Thread::i0,
		/* [9]  */ (addr_t Thread::*)&Thread::i1,
		/* [10] */ (addr_t Thread::*)&Thread::i2,
		/* [11] */ (addr_t Thread::*)&Thread::i3,
		/* [12] */ (addr_t Thread::*)&Thread::i4,
		/* [13] */ (addr_t Thread::*)&Thread::i5,
		/* [14] */ (addr_t Thread::*)&Thread::i6,
		/* [15] */ (addr_t Thread::*)&Thread::i7,
		/* [16] */ (addr_t Thread::*)&Thread::sp,
		/* [17] */ (addr_t Thread::*)&Thread::ip,
		/* [18] */ (addr_t Thread::*)&Thread::cpsr,
		/* [19] */ (addr_t Thread::*)&Thread::cpu_exception,
		/* [20] */ (addr_t Thread::*)&Thread::_fault_pd,
		/* [21] */ (addr_t Thread::*)&Thread::_fault_addr,
		/* [22] */ (addr_t Thread::*)&Thread::_fault_writes,
		/* [23] */ (addr_t Thread::*)&Thread::_fault_signal
	};
	return id < sizeof(_regs)/sizeof(_regs[0]) ? _regs[id] : 0;
}


Thread_event Thread::* Thread::_event(unsigned const id) const
{
	static Thread_event Thread::* _events[] = {
		/* [0] */ &Thread::_fault
	};
	return id < sizeof(_events)/sizeof(_events[0]) ? _events[id] : 0;
}


void Thread::_mmu_exception()
{
	_unschedule(AWAITS_RESUME);
	if (in_fault(_fault_addr, _fault_writes)) {
		_fault_pd    = (addr_t)_pd->platform_pd();
		_fault_signal = _fault.signal_context_id();
		_fault.submit();
		return;
	}
	PERR("unknown MMU exception");
}

void Thread::exception(unsigned const processor_id)
{
	switch (cpu_exception) {
	case RESET:
		return;
	default:
		PWRN("unknown exception");
		_stop();
	}
}

/*****************
 ** Kernel::Vm  **
 *****************/
void Kernel::Vm::exception(unsigned const processor_id)
{
	switch(_state->cpu_exception) {
	default:
		Processor_client::_unschedule();
		_context->submit(1);
	}
}

/*************************
 ** Kernel::Cpu_context **
 *************************/

void Kernel::Cpu_context::_init(size_t const stack_size) { }


/*************************
 ** CPU-state utilities **
 *************************/

typedef Thread_reg_id Reg_id;

static addr_t const _cpu_state_regs[] = {
	Reg_id::L0,   Reg_id::L1,   Reg_id::L2,  Reg_id::L3, Reg_id::L4,
	Reg_id::L5,   Reg_id::L6,   Reg_id::L7,  Reg_id::I0, Reg_id::I1,
	Reg_id::I2,   Reg_id::I3,   Reg_id::I4,  Reg_id::I5, Reg_id::I6,
	Reg_id::I7,   Reg_id::SP,   Reg_id::IP,  Reg_id::CPSR, Reg_id::CPU_EXCEPTION };

addr_t const * cpu_state_regs() { return _cpu_state_regs; }


size_t cpu_state_regs_length()
{
	return sizeof(_cpu_state_regs)/sizeof(_cpu_state_regs[0]);
}
