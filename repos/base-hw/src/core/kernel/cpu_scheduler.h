/*
 * \brief  Schedules CPU shares for the execution time of a CPU
 * \author Martin Stein
 * \date   2014-10-09
 */

/*
 * Copyright (C) 2014-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CORE__KERNEL__CPU_SCHEDULER_H_
#define _CORE__KERNEL__CPU_SCHEDULER_H_

/* core includes */
#include <util.h>
#include <util/misc_math.h>
#include <kernel/configuration.h>
#include <kernel/scheduler_policy.h>
#include <kernel/cpu_scheduler_util.h>
#include <kernel/double_list.h>
#include <kernel/cpu_scheduler_util.h>
#include <kernel/quota_scheduler.h>

namespace Kernel
{
	class Cpu_scheduler;
}

namespace Kernel {

	/**
	 * Schedules CPU shares for the execution time of a CPU
	 */
	class Cpu_scheduler;
}


class Kernel::Cpu_scheduler : public Scheduler_policy
{
	private:
		typedef Cpu_share                Share;
		typedef Cpu_fill                 Fill;
		typedef Cpu_claim                Claim;
		typedef Double_list_typed<Claim> Claim_list;
		typedef Double_list_typed<Fill>  Fill_list;
		typedef Cpu_priority             Prio;


	public:
		Cpu_scheduler(Share * const i, unsigned const q, unsigned const f) : Scheduler_policy(i, q, f) { }
};

#endif /* _CORE__KERNEL__CPU_SCHEDULER_H_ */
