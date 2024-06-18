/*
 * \brief  Utility to attach a dataspace to the local address space
 * \author Norman Feske
 * \date   2014-01-10
 */

/*
 * Copyright (C) 2014-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__BASE__ATTACHED_DATASPACE_H_
#define _INCLUDE__BASE__ATTACHED_DATASPACE_H_

#include <dataspace/client.h>
#include <base/env.h>

namespace Genode { class Attached_dataspace; }


class Genode::Attached_dataspace : Noncopyable
{
	public:

		struct Invalid_dataspace : Exception { };
		struct Region_conflict   : Exception { };

	private:

		Dataspace_capability _ds;

		Region_map &_rm;

		size_t const _size = { Dataspace_client(_ds).size() };

		Dataspace_capability _check(Dataspace_capability ds)
		{
			if (ds.valid())
				return ds;

			throw Invalid_dataspace();
		}

		addr_t _attach()
		{
			Region_map::Attr attr { };
			attr.writeable = true;
			return _rm.attach(_ds, attr).convert<addr_t>(
				[&] (Region_map::Range range) { return range.start; },
				[&] (Region_map::Attach_error e) -> addr_t {
					if (e == Region_map::Attach_error::OUT_OF_RAM)  throw Out_of_ram();
					if (e == Region_map::Attach_error::OUT_OF_CAPS) throw Out_of_caps();
					throw Region_conflict();
				});
		}

		addr_t _local_addr = _attach();

		/*
		 * Noncopyable
		 */
		Attached_dataspace(Attached_dataspace const &);
		Attached_dataspace &operator = (Attached_dataspace const &);

	public:

		/**
		 * Constructor
		 *
		 * \throw Region_conflict
		 * \throw Invalid_dataspace
		 * \throw Out_of_caps
		 * \throw Out_of_ram
		 */
		Attached_dataspace(Region_map &rm, Dataspace_capability ds)
		: _ds(_check(ds)), _rm(rm) { }

		/**
		 * Destructor
		 */
		~Attached_dataspace()
		{
			if (_local_addr)
				_rm.detach(_local_addr);
		}

		/**
		 * Return capability of the used dataspace
		 */
		Dataspace_capability cap() const { return _ds; }

		/**
		 * Request local address
		 *
		 * This is a template to avoid inconvenient casts at the caller.
		 * A newly attached dataspace is untyped memory anyway.
		 */
		template <typename T>
		T *local_addr() { return reinterpret_cast<T *>(_local_addr); }

		template <typename T>
		T const *local_addr() const { return reinterpret_cast<T const *>(_local_addr); }

		/**
		 * Return size
		 */
		size_t size() const { return _size; }

		/**
		 * Forget dataspace, thereby skipping the detachment on destruction
		 *
		 * This method can be called if the the dataspace is known to be
		 * physically destroyed, e.g., because the session where the dataspace
		 * originated from was closed. In this case, core will already have
		 * removed the memory mappings of the dataspace. So we have to omit the
		 * detach operation in '~Attached_dataspace'.
		 */
		void invalidate() { _local_addr = 0; }
};

#endif /* _INCLUDE__BASE__ATTACHED_DATASPACE_H_ */
