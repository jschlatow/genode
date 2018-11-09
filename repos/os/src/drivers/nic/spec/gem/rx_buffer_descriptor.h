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

#ifndef _INCLUDE__DRIVERS__NIC__GEM__RX_BUFFER_DESCRIPTOR_H_
#define _INCLUDE__DRIVERS__NIC__GEM__RX_BUFFER_DESCRIPTOR_H_

#include "buffer_descriptor.h"

using namespace Genode;


class Rx_buffer_descriptor : public Buffer_descriptor
{
	private:

		struct Addr : Register<0x00, 32> {
			struct Addr31to2 : Bitfield<2, 28> {};
			struct Wrap : Bitfield<1, 1> {};
			struct Package_available : Bitfield<0, 1> {};
		};
		struct Status : Register<0x04, 32> {
			struct Length : Bitfield<0, 13> {};
			struct Start_of_frame : Bitfield<14, 1> {};
			struct End_of_frame : Bitfield<15, 1> {};
		};

		enum { BUFFER_COUNT = 1024 };

		void _reset_descriptor(unsigned const i) {
			if (i > _max_index())
				return;

			/* clear status */
			_descriptors[i].status = 0;

			/* set physical buffer address and set not used by SW
			 * last descriptor must be marked by Wrap bit
			 */
			_descriptors[i].addr   = (_phys_addr_buffer(i) & Addr::Addr31to2::reg_mask())
			                       | Addr::Wrap::bits(i == _max_index());
		}

		inline bool _current_package_available()
		{
			return Addr::Package_available::get(_current_descriptor().addr);
		}

	public:
		Rx_buffer_descriptor(Genode::Env &env) : Buffer_descriptor(env, BUFFER_COUNT)
		{
			for (unsigned int i=0; i <= _max_index(); i++) {
				_reset_descriptor(i);
			}
		}

		bool next_packet()
		{
			for (unsigned int i=0; i < _max_index(); i++) {
				if (_current_package_available())
					return true;

				_increment_descriptor_index();
			}

			return false;
		}

		size_t package_length()
		{
			if (!_current_package_available())
				return 0;

			return Status::Length::get(_current_descriptor().status);
		}

		size_t get_package(char* const package, const size_t max_length)
		{
			if (!_current_package_available())
				return 0;

			const Status::access_t status = _current_descriptor().status;
			if (!Status::Start_of_frame::get(status) || !Status::End_of_frame::get(status)) {
				warning("Package split over more than one descriptor. Package ignored!");

				_reset_descriptor(_current_index());
				return 0;
			}

			const size_t length = Status::Length::get(status);
			if (length > max_length) {
				warning("Buffer for received package to small. Package ignored!");

				_reset_descriptor(_current_index());
				return 0;
			}

			const char* const src_buffer = _current_buffer();
			memcpy(package, src_buffer, length);

			_reset_descriptor(_current_index());

			return length;
		}

};

#endif /* _INCLUDE__DRIVERS__NIC__GEM__RX_BUFFER_DESCRIPTOR_H_ */
