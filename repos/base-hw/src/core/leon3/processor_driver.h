/*
 * \brief  CPU definition for LEON3
 * \author Johannes Schlatow
 * \date   2014-07-24
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _LEON3__PROCESSOR_DRIVER_H_
#define _LEON3__PROCESSOR_DRIVER_H_

/* core includes */
#include <processor_driver/leon3.h>

namespace Genode { class Processor_driver : public Leon3::Processor_driver { }; }

namespace Kernel { typedef Genode::Processor_driver Processor_driver; }

#endif /* _LEON3__PROCESSOR_DRIVER_H_ */

