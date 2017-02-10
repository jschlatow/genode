/*
 * \brief  MAC-address allocator
 * \author Martin Stein
 * \date   2016-10-24
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <nic_bridge/mac_allocator.h>

Net::Mac_address Net::Mac_allocator::mac_addr_base(0x03);
