/*
 * \brief  CPU driver for LEON3
 * \author Johannes Schlatow
 * \date   2014-07-24
 *
 * TODO implement LEON3 processor driver
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _PROCESSOR_DRIVER__LEON3_H_
#define _PROCESSOR_DRIVER__LEON3_H_

/* Genode includes */
#include <util/register.h>
#include <cpu/cpu_state.h>
#include <mmu.h>

/* local includes */
#include <board.h>
#include <util.h>

namespace Leon3
{
	using namespace Genode;

	/**
	 * CPU driver for core
	 */
	struct Processor_driver
	{
		enum {
//			TTBCR_N = 0,
			EXCEPTION_ENTRY = 0xffff0000,
			DATA_ACCESS_ALIGNM = 4,
		};

		/**
		 * Application specific register 17
		 */
		struct Asr17 : Register<32>
		{
			struct Nwin   : Bitfield<0, 5>  { }; /* number of register windows -1 */
			struct Nwp    : Bitfield<5, 3>  { }; /* number of watchpoints (0-4) */
			struct V8     : Bitfield<8, 1>  { }; /* Multiply and divide instructions available */
			struct M      : Bitfield<9, 1>  { }; /* MAC instruction available */
			struct Fpu    : Bitfield<10, 2> { }; /* FPU option */
			struct Ld     : Bitfield<12, 1> { }; /* load delay */
			struct Sv     : Bitfield<13, 1> { }; /* enable single vector trapping */
			struct Dw     : Bitfield<14, 1> { }; /* disable write error trap */
			struct Cf     : Bitfield<15, 2> { }; /* CPU clock frequence (CF+1 times AHB clock) */
			struct Cs     : Bitfield<17, 1> { }; /* clock switching enabled */
			struct Idx    : Bitfield<28, 4> { }; /* processor index  */

			/**
			 * Read register
			 */
			static access_t read()
			{
				access_t v;
				asm volatile ("mov %%asr17, %0" : "=r" (v) : : );
				return v;
			}

			/**
			 * Write register
			 */
			static void write(access_t const v) {
				asm volatile ("mov %0, %%asr17" : : "r" (v) : ); }
		};

		/**
		 * Window invalid mask
		 */
		struct Wim : Register<32>
		{
			/**
			 * Read register
			 */
			static access_t read()
			{
				access_t v;
				asm volatile ("rdwim %0" : "=r" (v) : : );
				return v;
			}

			/**
			 * Write register
			 */
			static void write(access_t const v) {
				asm volatile ("wrwim %0" : : "r" (v) : ); }
		};

		/**
		 * Trap base register
		 */
		struct Tbr : Register<32>
		{
			struct Type  : Bitfield<4, 8>  { }; /* trap type (read only) */
			struct Addr  : Bitfield<12,20> { }; /* trape base address */

			/**
			 * Read register
			 */
			static access_t read()
			{
				access_t v;
				asm volatile ("rdtbr %0" : "=r" (v) : : );
				return v;
			}

			/**
			 * Write register
			 */
			static void write(access_t const v) {
				asm volatile ("wrtbr %0" : : "r" (v) : ); }
		};

		/**
		 * Processor status register
		 */
		struct Psr : Register<32>
		{
			struct Cwp    : Bitfield<0, 5> { }; /* current window pointer */
			struct Et     : Bitfield<5, 1> { }; /* enable traps */
			struct Ps     : Bitfield<6, 1>      /* previous supervisor */
			{
				enum {
					USER       = 0x0,
					SUPERVISOR = 0x1,
				};
			};
			struct S      : Bitfield<7, 1>      /* supervisor */
			{
				enum {
					USER       = 0x0,
					SUPERVISOR = 0x1,
				};
			};
			struct Pil    : Bitfield<8, 4>      /* proc interrupt level */
			{
				enum {
					ALL    = 0x00,
					NONE   = 0x1F,
				};
			};
			struct Ef     : Bitfield<12,1> { }; /* enable floating point */
			struct Ec     : Bitfield<13,1> { }; /* enable co processor */
			struct Icc_c  : Bitfield<20,1> { }; /* integer condition code: carry */
			struct Icc_v  : Bitfield<21,1> { }; /* integer condition code: overflow */
			struct Icc_z  : Bitfield<22,1> { }; /* integer condition code: zero */
			struct Icc_n  : Bitfield<23,1> { }; /* integer condition code: negative */
			struct Ver    : Bitfield<24,4> { }; /* version */

			/**
			 * Read register
			 */
			static access_t read()
			{
				access_t v;
				asm volatile ("rdpsr %0" : "=r" (v) : : );
				return v;
			}

			/**
			 * Write register
			 */
			static void write(access_t const v) {
				asm volatile ("wrpsr %0\n\tnop;nop;nop;" : : "r" (v) : ); }

			/**
			 * Initial value for an userland execution context
			 */
			static access_t init_user()
			{
				return S::bits(S::USER) |
				       Pil::bits(Pil::ALL) |
				       Et::bits(1);
			}

			/**
			 * Initial value for the kernel execution context
			 */
			static access_t init_kernel()
			{
				return S::bits(S::SUPERVISOR) |
				       Pil::bits(Pil::NONE) |
				       Et::bits(1);
			}
		};

		/**
		 * Extend basic CPU state by members relevant for 'base-hw' only
		 */
		struct Context : Cpu_state
		{
			/**********************************************************
			 ** The offset and width of any of these classmembers is **
			 ** silently expected to be this way by several assembly **
			 ** files. So take care if you attempt to change them.   **
			 **********************************************************/

			uint32_t cidr;    /* context ID register backup */
			uint32_t t_table; /* base address of applied translation table */

			/**
			 * Get base of assigned translation lookaside buffer
			 */
			addr_t translation_table() const { return t_table; }

			/**
			 * Assign translation lookaside buffer
			 */
			void translation_table(addr_t const tt) { t_table = tt; }

			/**
			 * Assign protection domain
			 */
			void protection_domain(unsigned const id) { cidr = id; }
		};

		/**
		 * An usermode execution state
		 */
		struct User_context : Context
		{
			/**
			 * Constructor
			 */
			User_context();

			/***************************************************
			 ** Communication between user and context holder **
			 ***************************************************/

			void user_arg_0(unsigned const arg) { i0 = arg; }
			void user_arg_1(unsigned const arg) { i1 = arg; }
			void user_arg_2(unsigned const arg) { i2 = arg; }
			void user_arg_3(unsigned const arg) { i3 = arg; }
			void user_arg_4(unsigned const arg) { i4 = arg; }
			void user_arg_5(unsigned const arg) { i5 = arg; }
			void user_arg_6(unsigned const arg) { i6 = arg; }
			void user_arg_7(unsigned const arg) { i7 = arg; }
			unsigned user_arg_0() const { return i0; }
			unsigned user_arg_1() const { return i1; }
			unsigned user_arg_2() const { return i2; }
			unsigned user_arg_3() const { return i3; }
			unsigned user_arg_4() const { return i4; }
			unsigned user_arg_5() const { return i5; }
			unsigned user_arg_6() const { return i6; }
			unsigned user_arg_7() const { return i7; }

			/**
			 * Initialize thread context
			 *
			 * \param tt     physical base of appropriate translation table
			 * \param pd_id  kernel name of appropriate protection domain
			 */
			void init_thread(addr_t const tt, unsigned const pd_id)
			{
				cidr    = pd_id;
				t_table = tt;
			}

			/**
			 * Return if the context is in a page fault due to a translation miss
			 *
			 * \param va  holds the virtual fault-address if call returns 1
			 * \param w   holds wether it's a write fault if call returns 1
			 */
			bool in_fault(addr_t & va, addr_t & w) const
			{
//				/* determine fault type */
//				switch (cpu_exception) {
//
//				case PREFETCH_ABORT: {
//
//					/* check if fault was caused by a translation miss */
//					Ifsr::Fault_status const fs = Ifsr::fault_status();
//					if (fs == Ifsr::SECTION_TRANSLATION ||
//					    fs == Ifsr::PAGE_TRANSLATION)
//					{
//						/* fetch fault data */
//						w = 0;
//						va = ip;
//						return 1;
//					}
//					return 0; }
//
//				case DATA_ABORT: {
//
//					/* check if fault was caused by translation miss */
//					Dfsr::Fault_status const fs = Dfsr::fault_status();
//					if(fs == Dfsr::SECTION_TRANSLATION ||
//					   fs == Dfsr::PAGE_TRANSLATION)
//					{
//						/* fetch fault data */
//						Dfsr::access_t const dfsr = Dfsr::read();
//						w = Dfsr::Wnr::get(dfsr);
//						va = Dfar::read();
//						return 1;
//					}
//					return 0; }
//
//				default: return 0;
//				}
				PERR("in_fault() not implemented");
				return true;
			}
		};

		/**
		 * Configure this module appropriately for the first kernel run
		 */
		static void init_phys_kernel()
		{
			/* FIXME implement init_phys_kernel() */
		}

		/**
		 * Switch to the virtual mode in kernel
		 *
		 * \param section_table  section translation table of the initial
		 *                       address space this function switches to
		 * \param process_id     process ID of the initial address space
		 */
		static void init_virt_kernel(addr_t const section_table,
		                             unsigned const process_id)
		{
			/* FIXME implement init_phys_kernel() */
		}

		/**
		 * Ensure that TLB insertions get applied
		 */
		static void tlb_insertions() { flush_tlb(); }

		static void start_secondary_processors(void * const ip)
		{
			if (PROCESSORS > 1) { PERR("multiprocessing not implemented"); }
		}

		/**
		 * Invalidate all predictions about the future control-flow
		 */
		static void invalidate_control_flow_predictions()
		{
			/* FIXME invalidation of branch prediction not implemented */
		}

		/**
		 * Finish all previous data transfers
		 */
		static void data_synchronization_barrier()
		{
			/* FIXME data synchronization barrier not implemented */
		}

		/**
		 * Wait for the next interrupt as cheap as possible
		 */
		static void wait_for_interrupt()
		{
			/* FIXME cheap way of waiting is not implemented */
		}

		/**
		 * Return kernel name of the primary processor
		 */
		static unsigned primary_id() { return 0; }

		/**
		 * Return kernel name of the executing processor
		 */
		static unsigned executing_id() { return primary_id(); }

		/**
		 * Invalidate all entries of all instruction caches
		 */
		__attribute__((always_inline))
		static void invalidate_instr_caches()
		{
			PERR("invalidate_instr_caches() not implemented");
		}

		/**
		 * Flush all entries of all data caches
		 */
		inline static void flush_data_caches();

		/**
		 * Invalidate all entries of all data caches
		 */
		inline static void invalidate_data_caches();

		/**
		 * Flush all caches
		 */
		static void flush_caches()
		{
			flush_data_caches();
			invalidate_instr_caches();
		}

		/**
		 * Invalidate all TLB entries of one address space
		 *
		 * \param pid  ID of the targeted address space
		 */
		static void flush_tlb_by_pid(unsigned const pid)
		{
			PERR("flush_tlb_by_pid() not implemented");
			flush_caches();
		}

		/**
		 * Invalidate all TLB entries
		 */
		static void flush_tlb()
		{
			PERR("flush_tlb() not implemented");
			flush_caches();
		}

		/**
		 * Clean every data-cache entry within a virtual region
		 */
		static void
		flush_data_caches_by_virt_region(addr_t base, size_t const size)
		{
			PERR("flush_data_caches_by_virt_region() not implemented");
//			enum {
//				LINE_SIZE        = 1 << Board::CACHE_LINE_SIZE_LOG2,
//				LINE_ALIGNM_MASK = ~(LINE_SIZE - 1),
//			};
//			addr_t const top = base + size;
//			base = base & LINE_ALIGNM_MASK;
//			for (; base < top; base += LINE_SIZE) { Dccmvac::write(base); }
		}

		/**
		 * Invalidate every instruction-cache entry within a virtual region
		 */
		static void
		invalidate_instr_caches_by_virt_region(addr_t base, size_t const size)
		{
			PERR("flush_instr_caches_by_virt_region() not implemented");
//			enum {
//				LINE_SIZE        = 1 << Board::CACHE_LINE_SIZE_LOG2,
//				LINE_ALIGNM_MASK = ~(LINE_SIZE - 1),
//			};
//			addr_t const top = base + size;
//			base = base & LINE_ALIGNM_MASK;
//			for (; base < top; base += LINE_SIZE) { Icimvau::write(base); }
		}
	};
}

#endif /* _PROCESSOR_DRIVER__LEON3_H_ */

