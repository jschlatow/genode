/*
 * \brief  Platform driver - compound object for all derivate implementations
 * \author Johannes Schlatow
 * \author Stefan Kalkowski
 * \date   2022-05-10
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVERS__PLATFORM__COMMON_H_
#define _SRC__DRIVERS__PLATFORM__COMMON_H_

#include <base/registry.h>

#include <root.h>
#include <device_owner.h>
#include <control_device.h>

namespace Driver { class Common; };

class Driver::Common : Device_reporter,
                       public Device_owner
{
	private:

		Env                    & _env;
		String<64>               _rom_name;
		Attached_rom_dataspace   _devices_rom   { _env, _rom_name.string() };
		Attached_rom_dataspace   _platform_info { _env, "platform_info"    };
		Heap                     _heap          { _env.ram(), _env.rm()    };
		Sliced_heap              _sliced_heap   { _env.ram(), _env.rm()    };
		Device_model             _devices       { _env, _heap, *this       };
		Signal_handler<Common>   _dev_handler   { _env.ep(), *this,
		                                          &Common::_handle_devices };
		Device::Owner            _owner_id      { *this };

		Registry<Control_device>          _control_devices { };
		Registry<Control_device_factory>  _control_device_factories   { };

		Driver::Root             _root;

		Constructible<Expanding_reporter> _cfg_reporter { };
		Constructible<Expanding_reporter> _dev_reporter { };

		void _release_vanished_devices();
		void _handle_devices();
		bool _iommu();

	public:

		Common(Genode::Env                  & env,
		       Attached_rom_dataspace const & config_rom);

		Heap         & heap()    { return _heap;    }
		Device_model & devices() { return _devices; }

		Registry<Control_device_factory> & control_device_factories() {
			return _control_device_factories; }

		void announce_service();
		void handle_config(Xml_node config);
		void create_control_devices();


		/*********************
		 ** Device_reporter **
		 *********************/

		void update_report() override;
};


void Driver::Common::_release_vanished_devices()
{
	_control_devices.for_each([&] (Control_device & control_dev) {
		bool found = false;
		_devices.for_each([&] (Device const & dev) {
			if (found)
				return;

			if (control_dev.matches(dev))
				found = true;
		});

		if (!found)
			destroy(_heap, &control_dev);
	});
}


void Driver::Common::create_control_devices()
{
	_control_device_factories.for_each([&] (Control_device_factory & factory) {

		_devices.for_each([&] (Device & dev) {
			if (dev.owner().valid())
				return;

			if (factory.matches(dev)) {
				dev.acquire(*this);
				factory.create(_heap, _control_devices, dev);
			}
		});

	});
}


void Driver::Common::_handle_devices()
{
	_devices_rom.update();
	_devices.update(_devices_rom.xml());
	_release_vanished_devices();
	create_control_devices();
	update_report();
	_root.update_policy();
}


bool Driver::Common::_iommu()
{
	bool iommu = false;
	_platform_info.xml().with_optional_sub_node("kernel", [&] (Xml_node xml) {
		iommu = xml.attribute_value("iommu", false); });

	return iommu;
}


void Driver::Common::update_report()
{
	if (_dev_reporter.constructed())
		_dev_reporter->generate([&] (Xml_generator & xml) {
			_devices.generate(xml); });
}


void Driver::Common::handle_config(Xml_node config)
{
	config.for_each_sub_node("report", [&] (Xml_node const node) {
		_dev_reporter.conditional(node.attribute_value("devices", false),
		                          _env, "devices", "devices");
		_cfg_reporter.conditional(node.attribute_value("config", false),
		                          _env, "config", "config");
	});

	_root.update_policy();

	if (_cfg_reporter.constructed())
		_cfg_reporter->generate([&] (Xml_generator & xml) {
			config.with_raw_content([&] (char const *src, size_t len) {
				xml.append(src, len);
			});
		});
}


void Driver::Common::announce_service()
{
	_env.parent().announce(_env.ep().manage(_root));
}


Driver::Common::Common(Genode::Env                  & env,
                       Attached_rom_dataspace const & config_rom)
:
	_env(env),
	_rom_name(config_rom.xml().attribute_value("devices_rom",
	                                           String<64>("devices"))),
	_root(_env, _sliced_heap, config_rom, _devices, _iommu())
{
	_devices_rom.sigh(_dev_handler);
	_handle_devices();
}

#endif /* _SRC__DRIVERS__PLATFORM__COMMON_H_ */
