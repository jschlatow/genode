/*
 * \brief  Throughput benchmark component for Nic and Uplink sessions
 * \author Johannes Schlatow
 * \date   2022-06-14
 *
 * This component continously sends/receives UDP packets via a Nic or Uplink
 * session in order to benchmark the throughput.
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* local includes */
#include <interface.h>
#include <nic_root_component.h>
#include <nic_client.h>

/* Genode includes */
#include <base/component.h>
#include <base/heap.h>
#include <base/attached_rom_dataspace.h>
#include <util/arg_string.h>
#include <timer_session/connection.h>
#include <util/reconstructible.h>

namespace Nic_perf {
	class Main;

	using namespace Genode;
}


struct Nic_perf::Main
{
	Env                      &_env;

	Heap                      _heap   { _env.ram(), _env.rm() };

	Timer::Connection         _timer  { _env };

	Attached_rom_dataspace    _config { _env, "config" };

	unsigned                  _period_ms { 5000 };

	unsigned                  _count     { 10000 };

	Interface_registry        _registry { };

	Nic_perf::Nic_root        _root   { _env, _heap, _registry, _config };

	Constructible<Nic_client> _nic_client { };

	Genode::Signal_handler<Main> _config_handler =
		{ _env.ep(), *this, &Main::_handle_config };

	Genode::Signal_handler<Main> _timer_handler =
		{ _env.ep(), *this, &Main::_handle_timer };

	void _handle_config()
	{
		_config.update();

		_registry.for_each([&] (Interface &interface) {
			with_matching_policy(interface.label(), _config.xml(),
				[&] (Xml_node const &policy) {
					interface.apply_config(policy);
				},
				[&] () { /* no matches */
					interface.apply_config(Xml_node("<config/>"));
				}
			);
		});

		if (_nic_client.constructed())
			_nic_client.destruct();

		if (_config.xml().has_sub_node("nic-client"))
			_nic_client.construct(_env, _heap, _config.xml().sub_node("nic-client"), _registry);

		_period_ms = _config.xml().attribute_value("period_ms", _period_ms);
		_count     = _config.xml().attribute_value("count",     _count);

		if (_count)
			_timer.trigger_periodic(_period_ms * 1000);
	}

	void _handle_timer()
	{
		_registry.for_each([&] (Interface &interface) {
			Packet_stats &stats = interface.packet_stats();

			stats.calculate_throughput(_period_ms);
			log(stats);
			stats.reset();
		});

		_count--;
		if (!_count)
			_env.parent().exit(0);
	}

	Main(Env &env) : _env(env)
	{
		_env.parent().announce(_env.ep().manage(_root));

		_config.sigh(_config_handler);
		_timer.sigh(_timer_handler);

		_handle_config();
	}
};


void Component::construct(Genode::Env &env) { static Nic_perf::Main main(env); }

