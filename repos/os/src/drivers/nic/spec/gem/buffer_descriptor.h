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

#ifndef _INCLUDE__DRIVERS__NIC__GEM__BUFFER_DESCRIPTOR_H_
#define _INCLUDE__DRIVERS__NIC__GEM__BUFFER_DESCRIPTOR_H_

/* Genode includes */
#include <base/attached_ram_dataspace.h>
#include <util/mmio.h>

using namespace Genode;


class Buffer_descriptor : protected Attached_ram_dataspace, protected Mmio
{
	public:
		static const size_t BUFFER_DESC_SIZE = 0x08;
		static const size_t BUFFER_SIZE      = 1600;

	private:
		Attached_ram_dataspace _buffer_ds;

		size_t _buffer_count;

		size_t _descriptor_index;

		char* const _buffers;

	protected:
		typedef struct {
			uint32_t addr;
			uint32_t status;
		} descriptor_t;

		descriptor_t* const _descriptors;

		void _max_index(size_t max_index) { _buffer_count = max_index+1; };

		inline unsigned _max_index() { return _buffer_count-1; }

		inline void _increment_descriptor_index()
		{
			_descriptor_index = (_descriptor_index+1) % _buffer_count;
		}

		inline unsigned _current_index() { return _descriptor_index; }

		inline descriptor_t& _current_descriptor()
		{
			return _descriptors[_descriptor_index];
		}

		char * _current_buffer()
		{
			char * const buffer = &_buffers[BUFFER_SIZE * _descriptor_index];
			return buffer;
		}

		addr_t _phys_addr_buffer(const unsigned int index)
		{
			return Dataspace_client(_buffer_ds.cap()).phys_addr() + BUFFER_SIZE * index;
		}


	private:

		/*
		 * Noncopyable
		 */
		Buffer_descriptor(Buffer_descriptor const &);
		Buffer_descriptor &operator = (Buffer_descriptor const &);


	public:
		/*
		 * start of the ram spave contains all buffer descriptors
		 * after that the data spaces for the ethernet packages are following
		 */
		Buffer_descriptor(Genode::Env &env, const size_t buffer_count = 1)
		:
			Attached_ram_dataspace(env.ram(), env.rm(), BUFFER_DESC_SIZE * buffer_count, UNCACHED),
			Genode::Mmio( reinterpret_cast<addr_t>(local_addr<void>()) ),
			_buffer_ds(env.ram(), env.rm(), BUFFER_SIZE * buffer_count),
			_buffer_count(buffer_count),
			_descriptor_index(0),
			_buffers(_buffer_ds.local_addr<char>()),
			_descriptors(local_addr<descriptor_t>())
		{ }

		addr_t phys_addr() { return Dataspace_client(cap()).phys_addr(); }
};

#endif /* _INCLUDE__DRIVERS__NIC__GEM__BUFFER_DESCRIPTOR_H_ */
