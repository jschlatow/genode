/*
 * \brief  Test platform driver DMA allocator
 * \author Johannes Schlatow
 * \date   2024-05-29
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/component.h>
#include <base/log.h>
#include <base/heap.h>

#include <platform_session/dma_buffer.h>
#include <platform_session/device.h>

using namespace Genode;

struct Main
{
	Env      & env;

	Heap       heap { env.ram(), env.rm() };

	Reconstructible<Platform::Connection> platform { env };

	Constructible<Platform::Device>       device {};

	void alloc_dma_buffers()
	{
		for (unsigned idx = 0; idx < 64; idx++) {
			new (heap) Platform::Dma_buffer { *platform, 16*1024*1024, UNCACHED };
		}
	}

	Main(Env &env) : env(env)
	{
		/* allocate 1GB of DMA in 16MB chunks */
		alloc_dma_buffers();

		/* acquire dummy device with large area of reserved memory */
		device.construct(*platform, "dummy");
	}
};

void Component::construct(Genode::Env &env) { static Main main(env); }
