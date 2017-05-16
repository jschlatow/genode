/*
 * \brief  Quota Priority Scheduling
 * \author Dusting Frey
 * \date   2017-04-14
 */

#include <hw/assert.h>
#include <kernel/quota_priority_scheduler.h>

using namespace Kernel;


bool Quota_priority_scheduler::_fill_for_head()
{
    Share * s = _share(_fills.head());
	if (!s) { return 0; }

    _fills.for_each([&s] (Fill *const c) mutable {
            Share *si = _share(c);
            if (si->_prio > s->_prio) { s = si; }
    });

    _fills.remove(s);
    _fills.insert_head(s);

    _set_head(s, s->_fill, 0);
	return 1;
}

