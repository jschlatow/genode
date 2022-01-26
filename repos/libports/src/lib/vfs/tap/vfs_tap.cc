/*
 * \brief  Tap device emulation
 * \author Johannes Schlatow
 * \date   2022-01-21
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <net/mac_address.h>
#include <os/vfs.h>
#include <vfs/single_file_system.h>
#include <vfs/dir_file_system.h>
#include <vfs/readonly_value_file_system.h>
#include <vfs/value_file_system.h>
#include <util/xml_generator.h>
#include <util/reconstructible.h>

/* local includes */
#include "uplink_file_system.h"

namespace Vfs {
	enum Uplink_mode {
		NIC_CLIENT,
		UPLINK_CLIENT
	};
	static inline size_t ascii_to(char const *, Uplink_mode &);

	/* overload Value_file_system to work with Net::Mac_address */
	class Mac_file_system;

	/* main file system */
	class Tap_file_system;
}


class Vfs::Mac_file_system : public Value_file_system<Net::Mac_address>
{
	public:
		Mac_file_system(Name const & name, Net::Mac_address const & mac)
		: Value_file_system(name, mac)
		{ }

		using Value_file_system<Net::Mac_address>::value;

		Net::Mac_address value()
		{
			Net::Mac_address val { };
			Net::ascii_to(buffer().string(), val);

			return val;
		}
};

struct Vfs::Tap_file_system
{
	using Name  = String<64>;

	template <typename>
	struct Local_factory;

	template <typename>
	struct Data_file_system;

	template <typename>
	struct Compound_file_system;
};


Genode::size_t Vfs::ascii_to(char const *s, Uplink_mode &mode)
{

	if (!strcmp(s, "uplink", 6))         { mode = Uplink_mode::UPLINK_CLIENT; return 6;  }
	if (!strcmp(s, "uplink_client", 13)) { mode = Uplink_mode::UPLINK_CLIENT; return 13; }

	mode = Uplink_mode::NIC_CLIENT;
	return strlen(s);
}


/**
 * File system node for processing the packet data read/write
 */
template <typename FS>
class Vfs::Tap_file_system::Data_file_system : public FS
{
	private:

		using Local_vfs_handle  = typename FS::Vfs_handle;
		using Label             = typename FS::Vfs_handle::Label;
		using Registered_handle = Genode::Registered<Local_vfs_handle>;
		using Handle_registry   = Genode::Registry<Registered_handle>;
		using Open_result       = Directory_service::Open_result;

		Name             const &_name;
		Label            const &_label;
		Net::Mac_address const &_default_mac;
		Genode::Env            &_env;
		Handle_registry         _handle_registry { };

	public:

		Data_file_system(Genode::Env            & env,
		                 Name             const & name,
		                 Label            const & label,
		                 Net::Mac_address const & mac)
		:
			FS(name.string()),
			_name(name), _label(label), _default_mac(mac), _env(env)
		{ }

		/* must only be called if handle has been opened */
		Local_vfs_handle &device()
		{
			Local_vfs_handle *dev = nullptr;
			_handle_registry.for_each([&] (Local_vfs_handle &handle) {
				dev = &handle;
			 });

			struct Device_unavailable { };

			if (!dev)
				throw Device_unavailable();

			return *dev;
		}

		static const char *name()   { return "data"; }
		char const *type() override { return "data"; }

		Open_result open(char const  *path, unsigned flags,
		                 Vfs_handle **out_handle,
		                 Allocator   &alloc) override
		{
			if (!FS::_single_file(path))
				return Open_result::OPEN_ERR_UNACCESSIBLE;
			
			/* A tap device is exclusive open, thus return error if already opened. */
			unsigned handles = 0;
			_handle_registry.for_each([&handles] (Local_vfs_handle const &) {
				handles++;
			});
			if (handles) return Open_result::OPEN_ERR_EXISTS;

			try {
				*out_handle = new (alloc)
					Registered_handle(_handle_registry, _env, alloc, _label.string(),
					                  _default_mac, *this, *this, flags);
				return Open_result::OPEN_OK;
			}
			catch (Genode::Out_of_ram)  { return Open_result::OPEN_ERR_OUT_OF_RAM; }
			catch (Genode::Out_of_caps) { return Open_result::OPEN_ERR_OUT_OF_CAPS; }
		}


		/********************************
		 ** File I/O service interface **
		 ********************************/

		bool notify_read_ready(Vfs_handle *vfs_handle) override
		{
			Local_vfs_handle *handle =
				static_cast<Local_vfs_handle*>(vfs_handle);
			if (!handle)
				return false;

			handle->notifying(true);
			return true;
		}

		bool check_unblock(Vfs_handle * vfs_handle, bool rd, bool wr, bool) override
		{
			Local_vfs_handle *handle =
				static_cast<Local_vfs_handle*>(vfs_handle);

			return ((rd && handle && handle->read_ready()) || wr);
		}
};


template <typename FS>
struct Vfs::Tap_file_system::Local_factory : File_system_factory
{
	using Label       = typename FS::Vfs_handle::Label;
	using Name_fs     = Readonly_value_file_system<Name>;
	using Mac_addr_fs = Mac_file_system;

	Name             const _name;
	Label            const _label;
	Uplink_mode      const _mode;
	Net::Mac_address const _default_mac;
	Vfs::Env              &_env;
	Data_file_system<FS>   _data_fs { _env.env(), _name, _label, _default_mac };

	struct Info
	{
		Net::Mac_address _mac_addr;
		Name   const    &_name;

		Mac_addr_fs     &_mac_addr_fs;
		Name_fs         &_name_fs;

		Info(Mac_addr_fs            & mac_addr_fs,
		     Name_fs                & name_fs,
		     Net::Mac_address const & mac,
		     Name             const & name)
		: _mac_addr   (mac),
		  _name       (name),
		  _mac_addr_fs(mac_addr_fs),
		  _name_fs    (name_fs)
		{ }

		void print(Genode::Output &out) const
		{
			char buf[128] { };
			Genode::Xml_generator xml(buf, sizeof(buf), "tap", [&] () {
				xml.attribute("mac_addr", String<32>(_mac_addr));
				xml.attribute("name",                _name);
			});
			Genode::print(out, Genode::Cstring(buf));
		}

		void update()
		{
			_mac_addr_fs.value(_mac_addr);
			_name_fs    .value(_name);
		}
	};

	Mac_addr_fs                          _mac_addr_fs   { "mac_addr", _default_mac };
	Name_fs                              _name_fs       { "name",     _name };

	Info                                 _info          { _mac_addr_fs,
	                                                      _name_fs,
	                                                      _default_mac,
	                                                      _name };
	Readonly_value_file_system<Info>     _info_fs       { "info",     _info };

	/********************
	 ** Watch handlers **
	 ********************/

	Genode::Watch_handler<Vfs::Tap_file_system::Local_factory<FS>> _mac_addr_changed_handler {
		_mac_addr_fs, "/mac_addr",
		_env.alloc(),
		*this,
		&Vfs::Tap_file_system::Local_factory<FS>::_mac_addr_changed };

	void _mac_addr_changed()
	{
		Net::Mac_address new_mac = _mac_addr_fs.value();

		/* update of mac address only if changed */
		if (new_mac != _info._mac_addr)
			_data_fs.device().mac_address(new_mac);

		/* read back mac from device */
		_info._mac_addr = _data_fs.device().mac_address();

		/* propagate changes to fs */
		_info.update();
		_info_fs.value(_info);
	}

	/***********************
	 ** Factory interface **
	 ***********************/

	static Name name(Xml_node config)
	{
		return config.attribute_value("name", Name("tap"));
	}

	Local_factory(Vfs::Env &env, Xml_node config)
	:
		_name       (name(config)),
		_label      (config.attribute_value("label", Label(""))),
		_mode       (config.attribute_value("mode",  Uplink_mode::NIC_CLIENT)),
		_default_mac(config.attribute_value("mac",   Net::Mac_address { 0x02 })),
		_env(env)
	{ }

	Vfs::File_system *create(Vfs::Env&, Xml_node node) override
	{
		if (node.has_type("data"))      return &_data_fs;
		if (node.has_type("info"))      return &_info_fs;
		if (node.has_type("mac_addr"))  return &_mac_addr_fs;
		if (node.has_type("name"))      return &_name_fs;

		return nullptr;
	}
};


template <typename FS>
class Vfs::Tap_file_system::Compound_file_system : private Local_factory<FS>,
                                                   public  Vfs::Dir_file_system
{
	private:

		typedef Tap_file_system::Name Name;

		typedef String<200> Config;
		static Config _config(Name const &name)
		{
			char buf[Config::capacity()] { };

			/*
			 * By not using the node type "dir", we operate the
			 * 'Dir_file_system' in root mode, allowing multiple sibling nodes
			 * to be present at the mount point.
			 */
			Genode::Xml_generator xml(buf, sizeof(buf), "compound", [&] () {

				xml.node("data", [&] () {
					xml.attribute("name", name); });

				xml.node("dir", [&] () {
					xml.attribute("name", Name(".", name));
					xml.node("info",       [&] () {});
					xml.node("mac_addr",   [&] () {});
					xml.node("name",       [&] () {});
				});
			});

			return Config(Genode::Cstring(buf));
		}

	public:

		Compound_file_system(Vfs::Env &vfs_env, Genode::Xml_node node)
		:
			Local_factory<FS>(vfs_env, node),
			Vfs::Dir_file_system(vfs_env,
			                     Xml_node(_config(Local_factory<FS>::name(node)).string()),
			                     *this)
		{ }

		static const char *name() { return "tap"; }

		char const *type() override { return name(); }
};


extern "C" Vfs::File_system_factory *vfs_file_system_factory(void)
{
	struct Factory : Vfs::File_system_factory
	{
		Vfs::File_system *create(Vfs::Env &env, Genode::Xml_node config) override
		{
			if (config.attribute_value("mode", Vfs::Uplink_mode::NIC_CLIENT) == Vfs::Uplink_mode::NIC_CLIENT) {
				Genode::error("Nic mode not implemented");
				class Not_implemented { };
				throw Not_implemented { };
			}
			else
				return new (env.alloc())
					Vfs::Tap_file_system::Compound_file_system<Vfs::Uplink_file_system>(env, config);
		}
	};

	static Factory f;
	return &f;
}
