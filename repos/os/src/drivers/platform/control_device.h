/*
 * \brief  Platform driver - control device
 j \author Johannes Schlatow
 * \date   2023-01-20
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVERS__PLATFORM__CONTROL_DEVICE_H_
#define _SRC__DRIVERS__PLATFORM__CONTROL_DEVICE_H_

#include <base/registry.h>

#include <device.h>

namespace Driver
{
	using namespace Genode;

	struct Control_device;
	struct Control_device_factory;
}

class Driver::Control_device : private Genode::Registry<Control_device>::Element
{
	protected:

		Device::Name  _name;

	public:

		Control_device(Registry<Control_device> & registry,
		               Device::Name       const & name)
		: Registry<Control_device>::Element(registry, *this),
			_name(name)
		{ }

		bool matches(Device const & dev) {
			return dev.name() == _name; }
};

class Driver::Control_device_factory : private Genode::Registry<Control_device_factory>::Element
{
	protected:

		Device::Type  _type;

	public:

		Control_device_factory(Registry<Control_device_factory> & registry,
		                       Device::Type               const & type)
		: Registry<Control_device_factory>::Element(registry, *this),
		  _type(type)
		{ }

		virtual ~Control_device_factory() { }

		bool matches(Device const & dev) {
			return dev.type() == _type; }

		virtual void create(Allocator &,
		                    Registry<Control_device> &,
		                    Device const &) = 0;
};


#endif /* _SRC__DRIVERS__PLATFORM__CONTROL_DEVICE_H_ */
