/*
 * \brief  Base EMAC driver for the Xilinx EMAC PS used on Zynq devices
 * \author Timo Wischer
 * \date   2015-03-10
 */

/*
 * Copyright (C) 2015-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__DRIVERS__NIC__GEM__TX_BUFFER_DESCRIPTOR_H_
#define _INCLUDE__DRIVERS__NIC__GEM__TX_BUFFER_DESCRIPTOR_H_

#include <base/log.h>
#include <timer_session/connection.h>

#include "buffer_descriptor.h"

using namespace Genode;


class Tx_buffer_descriptor : public Buffer_descriptor
{
	private:
		enum { BUFFER_COUNT = 1024 };

		struct Addr : Register<0x00, 32> {};
		struct Status : Register<0x04, 32> {
			struct Length  : Bitfield<0, 14> {};
			struct Last_buffer  : Bitfield<15, 1> {};
			struct Wrap  : Bitfield<30, 1> {};
			struct Used  : Bitfield<31, 1> {};
			struct Chksum_err : Bitfield<20, 2> {};
		};

		class Package_send_timeout : public Genode::Exception {};

		Timer::Connection &_timer;

		void _reset_descriptor(unsigned const i) {
			if (i > _max_index())
				return;

			/* set physical buffer address */
			_descriptors[i].addr   = _phys_addr_buffer(i);

			/* set used by SW, also we do not use frame scattering */
			_descriptors[i].status = Status::Used::bits(1) |
			                         Status::Last_buffer::bits(1);

			/* last buffer must be marked by Wrap bit */
			if (i == _max_index())
				_descriptors[i].status |= Status::Wrap::bits(1);
		}

	public:

		Tx_buffer_descriptor(Genode::Env &env, Timer::Connection &timer)
		: Buffer_descriptor(env, BUFFER_COUNT), _timer(timer)
		{
			for (unsigned int i=0; i <= _max_index(); i++) {
				_reset_descriptor(i);
			}
		}

		void add_to_queue(const char* const packet, const size_t size)
		{
			if (size > BUFFER_SIZE) {
				warning("Ethernet package to big. Not sent!");
				return;
			}

			/* wait until the used bit is set (timeout after 10ms) */
			uint32_t timeout = 10000;
			while ( !Status::Used::get(_current_descriptor().status) ) {
				if (timeout == 0) {
					warning("Timed out waiting for tx buffer");
					throw Package_send_timeout();
				}
				timeout -= 10;

				/*  TODO buffer is full, instead of sleeping we should
				 *       therefore wait for tx_complete interrupt */
				_timer.usleep(10);
			}

			uint8_t chksum_err = Status::Chksum_err::get(_current_descriptor().status);
			if (chksum_err)
				Genode::log("Checksum offloading error", Genode::Hex(chksum_err));

			memcpy(_current_buffer(), packet, size);

			_current_descriptor().status &=  Status::Length::clear_mask();
			_current_descriptor().status |=  Status::Length::bits(size);

			/* unset the unset bit */
			_current_descriptor().status &=  Status::Used::clear_mask();

			_increment_descriptor_index();
		}
};

#endif /* _INCLUDE__DRIVERS__NIC__GEM__TX_BUFFER_DESCRIPTOR_H_ */
