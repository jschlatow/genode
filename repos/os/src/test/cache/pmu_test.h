/*
 * \brief  Test definitions using Cortex-A9 performance counters
 * \author Johannes Schlatow
 * \date   2022-03-08
 *
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__TEST__CACHE__PMU_TEST_H_
#define _SRC__TEST__CACHE__PMU_TEST_H_

enum PMU_TYPE : unsigned {
	ICACHE_MISS      = 0x01,  /* higher on linux */
	IUTLB_MISS       = 0x02,  /* higher on linux */
	DCACHE_MISS      = 0x03,  /* XXX significantly lower for linux memcpy */
	DCACHE_ACCESS    = 0x04,  /* very similar */
	DUTLB_MISS       = 0x05,  /* higher on linux */
	DATA_READ        = 0x06,  /* very similar */
	DATA_WRITE       = 0x07,  /* very similar */
	/* Cortex A9 events */
	COH_LINEFILL_MISS= 0x50,  /* XXX significantly lower for linux memcpy */
	COH_LINEFILL_HIT = 0x51,  /* higher for high KB and linux non-memcpy */
	INST_STALL       = 0x60,  /* higher for high KB on linux */
	DATA_STALL       = 0x61,  /* XXX vanish for linux memcpy */
	TLB_STALL        = 0x62,  /* higher for high KB on linux */
	DATA_EVICTIONS   = 0x65,  /* same */
	DATA_LINEFILLS   = 0x69,  /* 0 */
	PREFETCH_FILLS   = 0x6A,  /* 0 */
	PREFETCH_HITS    = 0x6B,  /* 0 */
	LOAD_STORE       = 0x70,  /* same */
	NEON_INST        = 0x74,  /* a few on linux */
	STALLED_PLD      = 0x80,  /* 0 */
	STALLED_WR       = 0x81,  /* significantly higher for linux memcpy and high KB */
	STALLED_ITLB     = 0x82,  /* higher for high KB on linux */
	STALLED_DTLB     = 0x83,  /* higher for high KB on linux, always low on genode */
	STALLED_IUTLB    = 0x84,
	STALLED_DUTLB    = 0x85,  /* higher for high KB on linux */
	ITLB_ALLOC       = 0x8D,  /* higher for high KB on linux */
	DTLB_ALLOC       = 0x8E,  /* higher for high KB on linux */
	EXT_IRQS         = 0x83,  /* higher on linux */
	PLE_LREQ_COMP    = 0xA0,  /* 0 */
	PLE_LREQ_SKIPPED = 0xA1,  /* 0 */
	PLE_FLUSH        = 0xA2,  /* 0 on genode */
	PLE_REQ_COMP     = 0xA3,  /* 0 */
	PLE_OVERFLOW     = 0xA4,  /* 0 */
	PLE_REQ_PROG     = 0xA5,  /* 0 */
};

void pmu_set_type(const unsigned counter, const PMU_TYPE type)
{
	// select counter #counter
	asm volatile ("MCR p15, 0, %0, C9, C12, 5" :: "r"(counter));

	// set type
	asm volatile ("MCR p15, 0, %0, C9, C13, 1" :: "r"(type));
}

void pmu_reset_and_enable(const PMU_TYPE t1,
                          const PMU_TYPE t2,
                          const PMU_TYPE t3,
                          const PMU_TYPE t4)
{
	// Program the performance-counter control-register
	enum { RESET_CC = 0x4, RESET_ALL = 0x2, ENABLE = 0x1 };

	const unsigned pmcr = RESET_ALL | RESET_CC | ENABLE;

	asm volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(pmcr));


	// Enable all counters
	enum { ENABLE_CC = 1 << 31, ENABLE_ALL = 0xF };

	const unsigned pmcenset = ENABLE_ALL | ENABLE_CC;

	asm volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(pmcenset));


	// Clear overflows
	asm volatile ("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(pmcenset));

	pmu_set_type(0, t1);
	pmu_set_type(1, t2);
	pmu_set_type(2, t3);
	pmu_set_type(3, t4);
}

void pmu_report(unsigned counter, const char * name)
{
	unsigned int counter_value;
	asm volatile ("MCR p15, 0, %0, C9, C12, 5" :: "r"(counter));
	asm volatile ("MRC p15, 0, %0, C9, C13, 2" :"=r"(counter_value));

	log(name, ": ", counter_value);
}

template <typename TEST>
unsigned long timed_test_pmu(void * src, void * dst, size_t sz, unsigned iterations, TEST && func)
{
	pmu_reset_and_enable(IUTLB_MISS,
	                     DUTLB_MISS, 
	                     ICACHE_MISS, 
	                     PLE_FLUSH);

	Time s { };

	for (; iterations; iterations--)
		func(src, dst, sz);

	{
		Time e { } ;
		Duration d = Time::duration(s, e);

		pmu_report(0, "IUTLB miss");
		pmu_report(1, "DUTLB miss");
		pmu_report(2, "ICACHE miss");
		pmu_report(3, "PLE FIFO flush");

		return d.value;
	}
}

#endif /* _SRC__TEST__CACHE__PMU_TEST_H_ */
