/**
 * \brief   Startup code for Genode applications on Sparc
 * \author  Johannes Schlatow
 * \date    2014-08-03
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */


/**************************
 ** .text (program code) **
 **************************/

.section ".text.crt0"

/* program entry-point */
.globl _start
_start:

	/* make initial value of some registers available to higher-level code */
    sethi %hi(__initial_sp), %l6
    st %sp, [%l6 + %lo(__initial_sp)]

	/*
	 * Install initial temporary environment that is replaced later by the
	 * environment that init_main_thread creates.
	 */
    sethi %hi(_stack_top), %sp
    or %sp, %lo(_stack_top), %sp

	/* if this is the dynamic linker, init_rtld relocates the linker */
	call init_rtld

	/* create proper environment for main thread */
	call init_main_thread

	/* apply environment that was created by init_main_thread */
    sethi %hi(init_main_thread_result), %l6
    ld [%l6 + %lo(init_main_thread_result)], %sp

	/* jump into init C code instead of calling it as it should never return */
	ba _main


/*********************************
 ** .bss (non-initialized data) **
 *********************************/

.section ".bss"

/* stack of the temporary initial environment */
	.align 8
_stack_low:
	.space (32 * 1024)-96
_stack_top:
    .space 96
_stack_high:
    .global __initial_sp
__initial_sp: /* initial value of the SP register */
    .space 4
