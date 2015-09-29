/*
 * \brief  Zedboard image memory
 * \author Mark Albers
 * \date   2015-04-21
 */

/*
 * Copyright (C) 2012-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__ZEDBOARD__IMAGE_MEM_H_
#define _INCLUDE__ZEDBOARD__IMAGE_MEM_H_

#include <ram_session/ram_session.h>
#include <dataspace/client.h>

using namespace Genode;

class Image_mem
{
    private:

        Ram_dataspace_capability _cap;
        void* _base;

    public:

        Image_mem(size_t size)
        {
            _cap = env()->ram_session()->alloc(size, UNCACHED);
            _base = env()->rm_session()->attach(_cap);
        }

        ~Image_mem() { env()->ram_session()->free(_cap); }

        Dataspace_capability dataspace() { return _cap; }
        addr_t phys_addr() const { return Dataspace_client(_cap).phys_addr(); }
        addr_t img_base() const { return (addr_t) _base; }
};

#endif /* _INCLUDE__ZEDBOARD__IMAGE_MEM_H_ */

