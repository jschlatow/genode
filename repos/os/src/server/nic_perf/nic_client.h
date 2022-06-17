/*
 * \brief  Nic client
 * \author Johannes Schlatow
 * \date   2022-06-16
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _NIC_CLIENT_H_
#define _NIC_CLIENT_H_

/* local includes */
#include <interface.h>

/* Genode includes */
#include <nic_session/connection.h>
#include <nic/packet_allocator.h>
#include <base/attached_rom_dataspace.h>

namespace Nic_perf {
	class Nic_client;

	using namespace Genode;
}


class Nic_perf::Nic_client : public Interface
{
	private:
		enum { BUF_SIZE = Nic::Session::QUEUE_SIZE * Nic::Packet_allocator::DEFAULT_PACKET_SIZE };

		Env                       &_env;
		Nic::Packet_allocator      _pkt_alloc;
		Nic::Connection            _nic { _env, &_pkt_alloc, BUF_SIZE, BUF_SIZE };
		Signal_handler<Nic_client> _packet_stream_handler
			{ _env.ep(), *this, &Nic_client::_handle_packet_stream };

		/***********************
		 * 'Interface' methods *
		 ***********************/

		void _send_alloc_pkt (Packet_descriptor &, void *&, size_t) override;
		void _send_submit_pkt(Packet_descriptor &) override;

	public:

		Nic_client(Env                 &env,
		           Genode::Allocator   &alloc,
		           Xml_node      const &policy,
		           Interface_registry  &registry)
		:
			Interface(registry, "nic-client", policy, false, Mac_address()),
			_env(env),
			_pkt_alloc(&alloc)
		{
			_nic.rx_channel()->sigh_ready_to_ack(_packet_stream_handler);
			_nic.rx_channel()->sigh_packet_avail(_packet_stream_handler);
			_nic.tx_channel()->sigh_ack_avail(_packet_stream_handler);
			_nic.tx_channel()->sigh_ready_to_submit(_packet_stream_handler);

			if (_generator.enabled()) _handle_packet_stream();
		}

		void _handle_packet_stream() {
			Interface::_handle_packet_stream(*_nic.tx(), *_nic.rx()); }
};


void Nic_perf::Nic_client::_send_alloc_pkt(Packet_descriptor &pkt,
                                           void *            &pkt_base,
                                           size_t             pkt_size)
{
	pkt      = _nic.tx()->alloc_packet(pkt_size);
	pkt_base = _nic.tx()->packet_content(pkt);
}


void Nic_perf::Nic_client::_send_submit_pkt(Packet_descriptor &pkt)
{
	_nic.tx()->try_submit_packet(pkt);
}

#endif /* _NIC_CLIENT_H_ */
