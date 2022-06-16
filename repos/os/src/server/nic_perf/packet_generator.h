/*
 * \brief  Packet generator
 * \author Johannes Schlatow
 * \date   2022-06-14
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _PACKET_GENERATOR_H_
#define _PACKET_GENERATOR_H_

/* Genode includes */
#include <util/xml_node.h>
#include <net/udp.h>
#include <net/ipv4.h>
#include <net/arp.h>
#include <net/mac_address.h>

namespace Nic_perf {
	using namespace Genode;
	using namespace Net;

	class Packet_generator;
}

class Nic_perf::Packet_generator
{
	public:
		struct Not_ready          : Exception { };
		struct Ip_address_not_set : Exception { };
		struct Udp_port_not_set   : Exception { };

	private:

		enum State { MUTED, NEED_ARP_REQUEST, WAIT_ARP_REPLY, READY };

		size_t       _mtu      { 1024 };
		bool         _enable   { false };
		Ipv4_address _dst_ip   { };
		Port         _dst_port { 0 };
		Mac_address  _dst_mac  { };
		State        _state    { MUTED };

		void _generate_arp_request(void *, Size_guard &, Mac_address const &, Ipv4_address const &);

		void _generate_test_packet(void *, Size_guard &, Mac_address const &, Ipv4_address const &);

	public:

		void apply_config(Xml_node const &config)
		{
			Ipv4_address old_ip = _dst_ip;

			/* restore defaults */
			_dst_ip   = Ipv4_address();
			_dst_port = Port(0);
			_enable   = false;
			_state    = MUTED;

			config.with_sub_node("tx", [&] (Xml_node node) {
				_mtu      = node.attribute_value("mtu",      _mtu);
				_dst_ip   = node.attribute_value("to",       _dst_ip);
				_dst_port = node.attribute_value("udp_port", _dst_port);
				_enable   = true;
				_state    = READY;
			});

			/* redo ARP resolution if dst ip changed */
			if (old_ip != _dst_ip) {
				_dst_mac = Mac_address();

				if (_enable)
					_state = NEED_ARP_REQUEST;
			}
		}

		bool enabled() const  { return _enable; }

		size_t size() const
		{
			switch (_state) {
			case READY:
				return _mtu;
			case NEED_ARP_REQUEST:
				return Ethernet_frame::MIN_SIZE + sizeof(uint32_t);
			case WAIT_ARP_REPLY:
			case MUTED:
				return 0;
			}

			return 0;
		}

		void dhcp_client_configured()
		{
			/* re-issue ARP request */
			if (_state == WAIT_ARP_REPLY)
				_state = NEED_ARP_REQUEST;
		}

		void handle_arp_reply(Arp_packet const & arp);

		void generate(void *, Size_guard &, Mac_address const &, Ipv4_address const &);
};


void Nic_perf::Packet_generator::handle_arp_reply(Arp_packet const & arp)
{
	if (arp.src_ip() != _dst_ip)
		return;

	if (_state != WAIT_ARP_REPLY)
		return;

	_dst_mac = arp.src_mac();
	_state = READY;
}


void Nic_perf::Packet_generator::_generate_arp_request(void               * pkt_base,
                                                       Size_guard         & size_guard,
                                                       Mac_address  const & from_mac,
                                                       Ipv4_address const & from_ip)
{
	if (from_ip == Ipv4_address()) {
		error("Ip address not set");
		throw Ip_address_not_set();
	}

	Ethernet_frame &eth = Ethernet_frame::construct_at(pkt_base, size_guard);
	eth.dst(Mac_address(0xff));
	eth.src(from_mac);
	eth.type(Ethernet_frame::Type::ARP);

	Arp_packet &arp = eth.construct_at_data<Arp_packet>(size_guard);
	arp.hardware_address_type(Arp_packet::ETHERNET);
	arp.protocol_address_type(Arp_packet::IPV4);
	arp.hardware_address_size(sizeof(Mac_address));
	arp.protocol_address_size(sizeof(Ipv4_address));
	arp.opcode(Arp_packet::REQUEST);
	arp.src_mac(from_mac);
	arp.src_ip(from_ip);
	arp.dst_mac(Mac_address(0xff));
	arp.dst_ip(_dst_ip);


}


void Nic_perf::Packet_generator::_generate_test_packet(void               * pkt_base,
                                                       Size_guard         & size_guard,
                                                       Mac_address  const & from_mac,
                                                       Ipv4_address const & from_ip)
{
	if (from_ip == Ipv4_address()) {
		error("Ip address not set");
		throw Ip_address_not_set();
	}

	if (_dst_port == Port(0)) {
		error("Udp port not set");
		throw Udp_port_not_set();
	}

	Ethernet_frame &eth = Ethernet_frame::construct_at(pkt_base, size_guard);
	eth.dst(_dst_mac);
	eth.src(from_mac);
	eth.type(Ethernet_frame::Type::IPV4);

	size_t const ip_off = size_guard.head_size();
	Ipv4_packet &ip = eth.construct_at_data<Ipv4_packet>(size_guard);
	ip.header_length(sizeof(Ipv4_packet) / 4);
	ip.version(4);
	ip.time_to_live(64);
	ip.protocol(Ipv4_packet::Protocol::UDP);
	ip.src(from_ip);
	ip.dst(_dst_ip);

	size_t udp_off = size_guard.head_size();
	Udp_packet &udp = ip.construct_at_data<Udp_packet>(size_guard);
	udp.src_port(Port(0));
	udp.dst_port(_dst_port);

	/* inflate packet up to _mtu */
	size_guard.consume_head(size_guard.unconsumed());

	/* fill in length fields and checksums */
	udp.length(size_guard.head_size() - udp_off);
	udp.update_checksum(ip.src(), ip.dst());
	ip.total_length(size_guard.head_size() - ip_off);
	ip.update_checksum();
}


void Nic_perf::Packet_generator::generate(void               * pkt_base,
                                          Size_guard         & size_guard,
                                          Mac_address  const & from_mac,
                                          Ipv4_address const & from_ip)
{
	switch (_state) {
		case READY:
			_generate_test_packet(pkt_base, size_guard, from_mac, from_ip);
			break;
		case NEED_ARP_REQUEST:
			_generate_arp_request(pkt_base, size_guard, from_mac, from_ip);
			_state = WAIT_ARP_REPLY;
			break;
		case MUTED:
		case WAIT_ARP_REPLY:
			throw Not_ready();
			break;
	}
}

#endif /* _PACKET_GENERATOR_H_ */
