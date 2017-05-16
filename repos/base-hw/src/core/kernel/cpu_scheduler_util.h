/*
 * \brief  Utility used by the scheduler
 * \author Dustin Frey
 * \date   2017-04-14
 */

#ifndef _CORE__INCLUDE__KERNEL__CPU_SCHEDULER_UTIL_H_
#define _CORE__INCLUDE__KERNEL__CPU_SCHEDULER_UTIL_H_


#include <kernel/double_list.h>
#include <util.h>
#include <kernel/configuration.h>

namespace Kernel {

	/**
	 * Priority of an unconsumed CPU claim versus other unconsumed CPU claims
	 */
	class Cpu_priority;

	/**
	 * Scheduling context that has quota and priority (low-latency)
	 */
	class Cpu_claim : public Double_list_item { };

	/**
	 * Scheduling context that has no quota or priority (best effort)
	 */
	class Cpu_fill  : public Double_list_item { };

	/**
	 * Scheduling context that is both claim and fill
	 */
	class Cpu_share;
}

class Kernel::Cpu_priority
{
	private:

		unsigned _value;

	public:

		enum {
			MIN = 0,
			MAX = cpu_priorities - 1,
		};

		/**
		 * Construct priority with value 'v'
		 */
		Cpu_priority(signed const v) : _value(Genode::min(v, MAX)) { }

		/*
		 * Standard operators
		 */

		Cpu_priority & operator =(signed const v)
		{
			_value = Genode::min(v, MAX);
			return *this;
		}

	operator signed() const { return _value; }
};

class Kernel::Cpu_share : public Cpu_claim, public Cpu_fill
{
	friend class Cpu_scheduler;
	friend class Quota_scheduler;

public:

	signed const _prio;
	unsigned     _quota;
	unsigned     _claim;
	unsigned     _fill;
	bool         _ready;

	public:

		/**
		 * Constructor
		 *
		 * \param p  claimed priority
		 * \param q  claimed quota
		 */
		Cpu_share(signed const p, unsigned const q)
		: _prio(p), _quota(q), _claim(q), _ready(0) { }

		/*
		 * Accessors
		 */

		bool ready() const { return _ready; }
		void quota(unsigned const q) { _quota = q; }
};



#endif
