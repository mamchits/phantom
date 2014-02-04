// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "ipv4.H"
#include "config_helper.H"

namespace pd {

template<>
bool in_t::helper_t<address_ipv4_t>::parse(
	ptr_t &ptr, address_ipv4_t &address, char const *, error_handler_t handler
) {
	ptr_t p = ptr;
	unsigned int a0, a1, a2, a3;

	if(
		p.parse(a3) && a3 < 0x100 && p.match<ident_t>('.') &&
		p.parse(a2) && a2 < 0x100 && p.match<ident_t>('.') &&
		p.parse(a1) && a1 < 0x100 && p.match<ident_t>('.') &&
		p.parse(a0) && a0 < 0x100
	) {
		uint32_t addr = (a3 << 24) + (a2 << 16) + (a1 << 8) + a0;

		address.value = addr;

		ptr = p;
		return true;
	}

	if(handler) (*handler)(ptr, "address_ipv4 syntax error");
	return false;
}

template<>
void out_t::helper_t<address_ipv4_t>::print(
	out_t &out, address_ipv4_t const &address, char const *
) {
	uint32_t ip_addr = address.value;

	out
		.print((ip_addr >> 24) & 0xff)('.')
		.print((ip_addr >> 16) & 0xff)('.')
		.print((ip_addr >> 8) & 0xff)('.')
		.print(ip_addr & 0xff)
	;
}

template<>
bool in_t::helper_t<network_ipv4_t>::parse(
	ptr_t &ptr, network_ipv4_t &network, char const *, error_handler_t handler
) {
	ptr_t p = ptr;
	address_ipv4_t address; unsigned int masklen;

	if(!p.parse(address))
		return false;

	if(!p.match<ident_t>('/') || !p.parse(masklen) || masklen > 32) {
		if(handler) (*handler)(ptr, "network_ipv4 syntax error #2");
		return false;
	}

	unsigned int shift = 32 - masklen;

	if(((address.value >> shift) << shift) != address.value) {
		if(handler) (*handler)(ptr, "network_ipv4 syntax error #1");
		return false;
	}

	network.prefix = address;
	network.shift = shift;

	ptr = p;

	return true;
}

template<>
void out_t::helper_t<network_ipv4_t>::print(
	out_t &out, network_ipv4_t const &network, char const *
) {
	out.print(network.prefix)('/').print(32 - network.shift);
}

namespace config {

template<>
void helper_t<address_ipv4_t>::parse(in_t::ptr_t &ptr, address_ipv4_t &address) {
	ptr.parse(address, error);
}

template<>
void helper_t<address_ipv4_t>::print(
	out_t &out, int, address_ipv4_t const &address
) {
	out.print(address);
}

template<>
void helper_t<address_ipv4_t>::syntax(out_t &out) {
	out(CSTR("<ipv4_address>"));
}

template<>
void helper_t<network_ipv4_t>::parse(in_t::ptr_t &ptr, network_ipv4_t &network) {
	ptr.parse(network, error);
}

template<>
void helper_t<network_ipv4_t>::print(
	out_t &out, int, network_ipv4_t const &network
) {
	out.print(network);
}

template<>
void helper_t<network_ipv4_t>::syntax(out_t &out) {
	out(CSTR("<ipv4_network>"));
}

} // namespace config

} // namespace pd
