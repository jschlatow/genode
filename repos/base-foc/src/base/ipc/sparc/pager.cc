/*
 * \brief  Fiasco.OC pager framework
 * \author Johannes Schlatow
 * \date   2014-08-04
 *
 * Sparc/Leon3 specific parts, when handling cpu-exceptions.
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/ipc_pager.h>


void Genode::Ipc_pager::get_regs(Thread_state *state)
{
	state->i0   = _regs.i0;
	state->i1   = _regs.i1;
	state->i2   = _regs.i2;
	state->i3   = _regs.i3;
	state->i4   = _regs.i4;
	state->i5   = _regs.i5;
	state->i6   = _regs.i6;
	state->i7   = _regs.i7;
	state->sp   = _regs.sp;
	state->ip   = _regs.ip;
	state->cpsr = _regs.psr;
	state->cpu_exception = _regs.err >> 20;
}


void Genode::Ipc_pager::set_regs(Thread_state state)
{
	_regs.i0    = state.i0;
	_regs.i1    = state.i1;
	_regs.i2    = state.i2;
	_regs.i3    = state.i3;
	_regs.i4    = state.i4;
	_regs.i5    = state.i5;
	_regs.i6    = state.i6;
	_regs.i7    = state.i7;
	_regs.sp    = state.sp;
	_regs.ip    = state.ip;
	_regs.psr   = state.cpsr;
}

