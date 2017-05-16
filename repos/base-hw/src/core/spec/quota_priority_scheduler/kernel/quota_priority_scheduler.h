
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

#ifndef _CORE__INCLUDE__KERNEL__QUOTA_PRIORITY_SCHEDULER_H
#define _CORE__INCLUDE__KERNEL__QUOTA_PRIORITY_SCHEDULER_H

/* core includes */
#include <util.h>
#include <kernel/configuration.h>
#include <kernel/cpu_scheduler_util.h>
#include <kernel/quota_scheduler.h>
#include <kernel/double_list.h>

#include <base/log.h>

namespace Kernel {

	/**
	 * Schedules CPU shares for the execution time of a CPU
	 */
	class Quota_priority_scheduler;
}


class Kernel::Quota_priority_scheduler : public Quota_scheduler
{
	private:
		typedef Cpu_share                Share;
		typedef Cpu_fill                 Fill;
		typedef Cpu_claim                Claim;
		typedef Double_list_typed<Claim> Claim_list;
		typedef Double_list_typed<Fill>  Fill_list;
		typedef Cpu_priority             Prio;

	public:
		Quota_priority_scheduler(Share * const i, unsigned const q, 
                unsigned const f) : Quota_scheduler(i, q, f) { }

    protected:
        bool _fill_for_head() override;
};

#endif
