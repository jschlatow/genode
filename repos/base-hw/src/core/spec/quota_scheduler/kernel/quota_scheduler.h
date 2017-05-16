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

#ifndef _CORE__INCLUDE__KERNEL__QUOTA_SCHEDULER_H
#define _CORE__INCLUDE__KERNEL__QUOTA_SCHEDULER_H

/* core includes */
#include <util.h>
#include <util/misc_math.h>
#include <kernel/configuration.h>
#include <kernel/cpu_scheduler_util.h>
#include <kernel/double_list.h>

namespace Kernel {

	/**
	 * Schedules CPU shares for the execution time of a CPU
	 */
	class Quota_scheduler;
}


class Kernel::Quota_scheduler
{
    protected:
		typedef Cpu_share                Share;
		typedef Cpu_fill                 Fill;
		typedef Cpu_claim                Claim;
		typedef Double_list_typed<Claim> Claim_list;
		typedef Double_list_typed<Fill>  Fill_list;
		typedef Cpu_priority             Prio;

		Claim_list     _rcl[Prio::MAX + 1]; /* ready claims */
		Claim_list     _ucl[Prio::MAX + 1]; /* unready claims */
		Fill_list      _fills;              /* ready fills */
		Share * const  _idle;
		Share *        _head;
		unsigned       _head_quota;
		bool           _head_claims;
		bool           _head_yields;
		unsigned const _quota;
		unsigned       _residual;
		unsigned const _fill;

		template <typename F> void _for_each_prio(F f) {
			for (signed p = Prio::MAX; p > Prio::MIN - 1; p--) { f(p); } }

		template <typename T>
		static Share * _share(T * const t) { return static_cast<Share *>(t); }

		static void _reset(Claim * const c);

		void     _reset_claims(unsigned const p);
		void     _next_round();
		void     _consumed(unsigned const q);
		void     _set_head(Share * const s, unsigned const q, bool const c);
		void     _next_fill();
		void     _head_claimed(unsigned const r);
		void     _head_filled(unsigned const r);
		bool     _claim_for_head();
		virtual bool     _fill_for_head();
		unsigned _trim_consumption(unsigned & q);

		/**
		 * Fill 's' becomes a claim due to a quota donation
		 */
		void _quota_introduction(Share * const s);

		/**
		 * Claim 's' looses its state as claim due to quota revokation
		 */
		void _quota_revokation(Share * const s);

		/**
		 * The quota of claim 's' changes to 'q'
		 */
		void _quota_adaption(Share * const s, unsigned const q);

	public:

		/**
		 * Constructor
		 *
		 * \param i  Gets scheduled with static quota when no other share
		 *           is schedulable. Unremovable. All values get ignored.
		 * \param q  total amount of time quota that can be claimed by shares
		 * \param f  time-slice length of the fill round-robin
		 */
		Quota_scheduler(Share * const i, unsigned const q, unsigned const f);

		/**
		 * Update head according to the consumption of quota 'q'
		 */
		void update(unsigned q);

		/**
		 * Set 's1' ready and return wether this outdates current head
		 */
		bool ready_check(Share * const s1);

		/**
		 * Set share 's' ready
		 */
		void ready(Share * const s);

		/**
		 * Set share 's' unready
		 */
		void unready(Share * const s);

		/**
		 * Current head looses its current claim/fill for this round
		 */
		void yield();

		/**
		 * Remove share 's' from scheduler
		 */
		void remove(Share * const s);

		/**
		 * Insert share 's' into scheduler
		 */
		void insert(Share * const s);

		/**
		 * Set quota of share 's' to 'q'
		 */
		void quota(Share * const s, unsigned const q);

		/*
		 * Accessors
		 */

		Share * head() const { return _head; }
		unsigned head_quota() const {
			return Genode::min(_head_quota, _residual); }
		unsigned quota() const { return _quota; }
        unsigned residual() const { return _residual; }
};

#endif /* _CORE__INCLUDE__KERNEL__CPU_SCHEDULER_H_ */
