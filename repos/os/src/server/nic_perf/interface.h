/*
 * \brief  Base class for Nic/Uplink session components
 * \author Johannes Schlatow
 * \date   2022-06-15
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

/* local includes */
#include <packet_generator.h>
#include <packet_stats.h>

/* Genode includes */
#include <base/registry.h>
#include <os/packet_stream.h>
#include <net/mac_address.h>
#include <net/ipv4.h>
#include <net/udp.h>
#include <net/dhcp.h>

namespace Nic_perf {
	using namespace Genode;
	using namespace Net;

	using Dhcp_options = Dhcp_packet::Options_aggregator<Size_guard>;

	class Interface;

	using Interface_registry = Registry<Interface>;
}

class Nic_perf::Interface
{
	protected:

		Interface_registry::Element _element;
		Session_label               _label;

		Packet_stats                _stats;
		Packet_generator            _generator;

		bool                        _mac_from_policy;

		Mac_address                 _mac { };
		Mac_address const           _default_mac;
		Ipv4_address                _ip  { };
		Ipv4_address                _dhcp_client_ip { };

		static Ipv4_address _subnet_mask()
		{
			uint8_t buf[] = { 0xff, 0xff, 0xff, 0 };
			return Ipv4_address((void*)buf);
		}

		void _handle_eth(void *, size_t);
		void _handle_ip(Ethernet_frame &, Size_guard &);
		void _handle_arp(Ethernet_frame &, Size_guard &);
		void _handle_dhcp_request(Ethernet_frame &, Dhcp_packet &);
		void _send_dhcp_reply(Ethernet_frame const &, Dhcp_packet const &, Dhcp_packet::Message_type);

		virtual void _send_alloc_pkt (Packet_descriptor &, void *&, size_t) = 0;
		virtual void _send_submit_pkt(Packet_descriptor &) = 0;

		template <typename FUNC>
		bool _send(size_t, FUNC &&);

		template <typename SOURCE, typename SINK>
		void _handle_packet_stream(SOURCE &, SINK &);

	public:

		virtual ~Interface() { };

		Interface(Interface_registry  &registry,
		          Session_label const &label,
		          Xml_node      const &policy,
		          bool                 mac_from_policy,
		          Mac_address          mac)
		: _element(registry, *this),
		  _label(label),
		  _stats(_label),
		  _generator(),
		  _mac_from_policy(mac_from_policy),
		  _default_mac(mac)
		{ apply_config(policy); }

		void apply_config(Xml_node const &config)
		{
			_generator.apply_config(config);

			/* restore defaults when applied to empty/incomplete config */
			_mac            = _default_mac;
			_ip             = Ipv4_address();
			_dhcp_client_ip = Ipv4_address();

			config.with_sub_node("interface", [&] (Xml_node node) {
				_ip             = node.attribute_value("ip", _ip);
				_dhcp_client_ip = node.attribute_value("dhcp_client_ip", _dhcp_client_ip);

				if (_mac_from_policy)
					_mac         = node.attribute_value("mac", _mac);
			});
		}

		Session_label const &label()        const { return _label; }
		Packet_stats        &packet_stats()       { return _stats; }
};


void Nic_perf::Interface::_handle_eth(void * pkt_base, size_t size)
{
	try {
		Size_guard size_guard(size);
		Ethernet_frame &eth = Ethernet_frame::cast_from(pkt_base, size_guard);
		switch (eth.type()) {
			case Ethernet_frame::Type::ARP:
				_handle_arp(eth, size_guard);
				break;
			case Ethernet_frame::Type::IPV4:
				_handle_ip(eth, size_guard);
				break;
			default:
				;
		}
	} catch (Size_guard::Exceeded) {
		warning("Size guard exceeded");
	}

	_stats.rx_packet(size);
}


void Nic_perf::Interface::_handle_arp(Ethernet_frame & eth, Size_guard & size_guard)
{
	Arp_packet &arp = eth.data<Arp_packet>(size_guard);
	if (!arp.ethernet_ipv4())
		return;

	Ipv4_address old_src_ip { };

	switch (arp.opcode()) {
	case Arp_packet::REPLY:
		_generator.handle_arp_reply(arp);

		break;

	case Arp_packet::REQUEST:
		/* check whether the request targets us */
		if (arp.dst_ip() != _ip)
			return;

		old_src_ip = arp.src_ip();
		arp.opcode(Arp_packet::REPLY);
		arp.dst_mac(arp.src_mac());
		arp.src_mac(_mac);
		arp.src_ip(arp.dst_ip());
		arp.dst_ip(old_src_ip);
		eth.dst(arp.dst_mac());
		eth.src(_mac);

		_send(size_guard.total_size(), [&] (void * pkt_base, Size_guard & size_guard) {
			memcpy(pkt_base, (void*)&eth, size_guard.total_size());
		});
		break;

	default:
		;
	}
}


void Nic_perf::Interface::_handle_ip(Ethernet_frame & eth, Size_guard & size_guard)
{
	Ipv4_packet &ip = eth.data<Ipv4_packet>(size_guard);
	if (ip.protocol() == Ipv4_packet::Protocol::UDP) {

		Udp_packet &udp = ip.data<Udp_packet>(size_guard);
		if (Dhcp_packet::is_dhcp(&udp)) {
			Dhcp_packet &dhcp = udp.data<Dhcp_packet>(size_guard);
			if (dhcp.op() == Dhcp_packet::REQUEST)
				_handle_dhcp_request(eth, dhcp);
		}
	}
}


void Nic_perf::Interface::_handle_dhcp_request(Ethernet_frame & eth, Dhcp_packet & dhcp)
{
	Dhcp_packet::Message_type const msg_type = 
		dhcp.option<Dhcp_packet::Message_type_option>().value();

	switch (msg_type) {
	case Dhcp_packet::Message_type::DISCOVER:
		_send_dhcp_reply(eth, dhcp, Dhcp_packet::Message_type::OFFER);
		break;
	case Dhcp_packet::Message_type::REQUEST:
		_send_dhcp_reply(eth, dhcp, Dhcp_packet::Message_type::ACK);
		_generator.dhcp_client_configured();
		break;
	default:
		;
	}
}


void Nic_perf::Interface::_send_dhcp_reply(Ethernet_frame      const & eth_req,
                                           Dhcp_packet         const & dhcp_req,
                                           Dhcp_packet::Message_type   msg_type)
{
	if (_ip == Ipv4_address())
		return;

	if (_dhcp_client_ip == Ipv4_address())
		return;

	enum { PKT_SIZE = 512 };
	_send(PKT_SIZE, [&] (void *pkt_base,  Size_guard &size_guard) {

		Ethernet_frame &eth = Ethernet_frame::construct_at(pkt_base, size_guard);
		if (msg_type == Dhcp_packet::Message_type::OFFER) {
			eth.dst(Ethernet_frame::broadcast()); }
		else {
			eth.dst(eth_req.src()); }
		eth.src(_mac);
		eth.type(Ethernet_frame::Type::IPV4);

		/* create IP header of the reply */
		size_t const ip_off = size_guard.head_size();
		Ipv4_packet &ip = eth.construct_at_data<Ipv4_packet>(size_guard);
		ip.header_length(sizeof(Ipv4_packet) / 4);
		ip.version(4);
		ip.time_to_live(64);
		ip.protocol(Ipv4_packet::Protocol::UDP);
		ip.src(_ip);
		ip.dst(_dhcp_client_ip);

		/* create UDP header of the reply */
		size_t const udp_off = size_guard.head_size();
		Udp_packet &udp = ip.construct_at_data<Udp_packet>(size_guard);
		udp.src_port(Port(Dhcp_packet::BOOTPS));
		udp.dst_port(Port(Dhcp_packet::BOOTPC));

		/* create mandatory DHCP fields of the reply  */
		Dhcp_packet &dhcp = udp.construct_at_data<Dhcp_packet>(size_guard);
		dhcp.op(Dhcp_packet::REPLY);
		dhcp.htype(Dhcp_packet::Htype::ETH);
		dhcp.hlen(sizeof(Mac_address));
		dhcp.xid(dhcp_req.xid());
		if (msg_type == Dhcp_packet::Message_type::INFORM) {
			dhcp.ciaddr(_dhcp_client_ip); }
		else {
			dhcp.yiaddr(_dhcp_client_ip); }
		dhcp.siaddr(_ip);
		dhcp.client_mac(dhcp_req.client_mac());
		dhcp.default_magic_cookie();

		/* append DHCP option fields to the reply */
		Dhcp_packet::Options_aggregator<Size_guard> dhcp_opts(dhcp, size_guard);
		dhcp_opts.append_option<Dhcp_packet::Message_type_option>(msg_type);
		dhcp_opts.append_option<Dhcp_packet::Server_ipv4>(_ip);
		dhcp_opts.append_option<Dhcp_packet::Ip_lease_time>(86400);
		dhcp_opts.append_option<Dhcp_packet::Subnet_mask>(_subnet_mask());
		dhcp_opts.append_option<Dhcp_packet::Router_ipv4>(_ip);

		dhcp_opts.append_dns_server([&] (Dhcp_options::Dns_server_data &data) {
			data.append_address(_ip);
		});
		dhcp_opts.append_option<Dhcp_packet::Broadcast_addr>(Ipv4_packet::broadcast());
		dhcp_opts.append_option<Dhcp_packet::Options_end>();

		/* fill in header values that need the packet to be complete already */
		udp.length(size_guard.head_size() - udp_off);
		udp.update_checksum(ip.src(), ip.dst());
		ip.total_length(size_guard.head_size() - ip_off);
		ip.update_checksum();
	});
}


template <typename FUNC>
bool Nic_perf::Interface::_send(size_t pkt_size, FUNC && write_to_pkt)
{
	if (!pkt_size)
		return false;

	try {
		Packet_descriptor  pkt;
		void              *pkt_base;

		_send_alloc_pkt(pkt, pkt_base, pkt_size);
		Size_guard size_guard { pkt_size };
		write_to_pkt(pkt_base, size_guard);

		_send_submit_pkt(pkt);
	} catch (...) { return false; }

	_stats.tx_packet(pkt_size);

	return true;
}

template <typename SOURCE, typename SINK>
void Nic_perf::Interface::_handle_packet_stream(SOURCE & source, SINK & sink)
{
	/* TODO trace burst length */

	/* handle acks from client */
	while (source.ack_avail())
		source.release_packet(source.try_get_acked_packet());

	/* loop while we can make Rx progress */
	for (;;) {
		if (!sink.ready_to_ack())
			break;

		if (!sink.packet_avail())
			break;

		Packet_descriptor const packet_from_client = sink.try_get_packet();

		if (sink.packet_valid(packet_from_client)) {
			_handle_eth(sink.packet_content(packet_from_client), packet_from_client.size());
			if (!sink.try_ack_packet(packet_from_client)) {
				/* XXX remove */
				error("ACK queue saturated");
				break;
			}
		}
	}

	/* skip sending if disabled */
	if (!_generator.enabled()) {
		sink.wakeup();
		source.wakeup();
		return;
	}

	/* loop while we can make Tx progress */
	for (;;) {
		/*
		 * The client fails to pick up the packets from the rx channel. So we
		 * won't try to submit new packets.
		 */
		if (!source.ready_to_submit())
			break;

		bool okay =
			_send(_generator.size(), [&] (void * pkt_base, Size_guard & size_guard) {
				_generator.generate(pkt_base, size_guard, _mac, _ip);
			});

		if (!okay)
			break;
	}

	sink.wakeup();
	source.wakeup();
}

#endif /* _INTERFACE_H_ */
