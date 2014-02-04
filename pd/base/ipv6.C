// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "ipv6.H"
#include "ipv4.H"
#include "config_helper.H"

namespace pd {

static network_ipv6_t ipv6_ipv4_map = network_ipv6_t(
	address_ipv6_t((uint128_t)0xffff << 32), 32
);

static inline uint16_t __ipv6_digit(uint128_t ip_addr, unsigned int number) {
	return (ip_addr >> (16 * (7 - number))) & 0xffff;
}

template<>
bool in_t::helper_t<address_ipv6_t>::parse(
	ptr_t &ptr, address_ipv6_t &address, char const *, error_handler_t handler
) {
	unsigned short digits[8];
	int ind = 0;
	int dcolon = 8;

	{
		ptr_t p = ptr;

		p.match<lower_t>(CSTR("::ffff:"));

		for(ptr_t _p = p; _p; ++_p) {
			char c = *_p;

			if(c == '.') {
				address_ipv4_t v4_addr;

				if(!p.parse(v4_addr, handler))
					return false;

				address = address_ipv6_t(ipv6_ipv4_map.prefix.value | v4_addr.value);

				ptr = p;
				return true;
			}
			else if(c < '0' || c > '9')
				break;
		}
	}

	ptr_t p = ptr;

	if(!p) {
		if(handler) (*handler)(ptr, "address_ipv6 syntax error #0");
		return false;
	}

	bool digit_required = true;

	if(*p == ':') {
		++p;
		if(!p || *p != ':') {
			if(handler) (*handler)(ptr, "address_ipv6 syntax error #1");
			return false;
		}

		++p;
		dcolon = 0;
		digit_required = false;
	}

	while(true) {
		switch(p ? *p : '\0') {
			case '0'...'9': case 'a'...'f': case 'A'...'F': break;
			default: {
				if(digit_required) {
					if(handler) (*handler)(ptr, "address_ipv6 syntax error #3");
					return false;
				}

				goto _out;
			}
		}

		if(!p.parse(digits[ind], "x")) {
			if(handler) (*handler)(ptr, "address_ipv6 syntax error #5");
			return false;
		}

		++ind;

		if(!p || *p != ':')
			break;

		if(ind == 8) {
			if(handler) (*handler)(ptr, "address_ipv6 syntax error #2");
			return false;
		}

		digit_required = true;

		++p;

		if((p ? *p : '\0') == ':') {
			if(dcolon != 8) {
				if(handler) (*handler)(ptr, "address_ipv6 syntax error #4");
				return false;
			}

			++p;
			dcolon = ind;
			digit_required = false;
		}
	}
_out:

	if(dcolon == 8 && ind < 8) {
		if(handler) (*handler)(ptr, "address_ipv6 syntax error #6");
		return false;
	}

	uint128_t ip_addr = 0;

	for(int i = 0; i < dcolon; ++i)
		ip_addr = (ip_addr << 16) | digits[i];

	ip_addr <<= (16 * (8 - ind));

	for(int i = dcolon; i < ind; ++i)
		ip_addr = (ip_addr << 16) | digits[i];

	address = address_ipv6_t(ip_addr);
	ptr = p;

	return true;
}

template<>
void out_t::helper_t<address_ipv6_t>::print(
	out_t &out, address_ipv6_t const &address, char const *fmt
) {
	bool raw = (fmt && *fmt == 'r');

	if(!raw && ipv6_ipv4_map.match(address)) {
		out.print(address_ipv4_t(address.value & 0xffffffff));
		return;
	}

	uint128_t ip_addr = address.value;

	uint16_t digits[8] = {
		__ipv6_digit(ip_addr, 0), __ipv6_digit(ip_addr, 1),
		__ipv6_digit(ip_addr, 2), __ipv6_digit(ip_addr, 3),
		__ipv6_digit(ip_addr, 4), __ipv6_digit(ip_addr, 5),
		__ipv6_digit(ip_addr, 6), __ipv6_digit(ip_addr, 7),
	};

	if(raw) {
		out
			.print(digits[0], "04x")(':').print(digits[1], "04x")(':')
			.print(digits[2], "04x")(':').print(digits[3], "04x")(':')
			.print(digits[4], "04x")(':').print(digits[5], "04x")(':')
			.print(digits[6], "04x")(':').print(digits[7], "04x")
		;
	}
	else {
		unsigned int zb = 8, zl = 0, _zb = 8, _zl = 0;

		for(unsigned int i = 0; i < 8; ++i) {
			if(!digits[i]) {
				if(_zb == 8) {
					_zb = i; _zl = 1;
				}
				else {
					++_zl;
				}
			}
			else {
				if(_zb < 8) {
					if(_zl > zl) {
						zb = _zb; zl = _zl;
					}
					_zb = 8; _zl = 0;
				}
			}
		}

		if(_zb < 8 && _zl > zl) {
			zb = _zb; zl = _zl;
		}

		bool nc = false;
		for(unsigned int i = 0; i < zb; ++i) {
			if(nc) out(':');
			out.print(digits[i], "x");
			nc = true;
		}

		if(zb < 8)
			out(':')(':');

		nc = false;
		for(unsigned int i = zb + zl; i < 8; ++i) {
			if(nc) out(':');
			out.print(digits[i], "x");
			nc = true;
		}
	}
}

template<>
bool in_t::helper_t<network_ipv6_t>::parse(
	ptr_t &ptr, network_ipv6_t &network, char const *, error_handler_t handler
) {
	ptr_t p = ptr;
	address_ipv6_t address; unsigned int masklen, shift;

	for(ptr_t _p = p; _p; ++_p) {
		char c = *_p;

		if(c == '.') {
			network_ipv4_t v4_net;
			if(!p.parse(v4_net, handler))
				return false;

			network.prefix = address_ipv6_t(ipv6_ipv4_map.prefix.value | v4_net.prefix.value);
			network.shift = v4_net.shift;

			ptr = p;

			return true;
		}

		if(c < '0' || c > '9')
			break;
	}

	if(!p.parse(address, handler))
		return false;

	if(!p.match<ident_t>('/') || !p.parse(masklen) || masklen > 128) {
		if(handler) (*handler)(ptr, "network_ipv6 syntax error #2");
		return false;
	}

	shift = 128 - masklen;

	if(((address.value >> shift) << shift) != address.value) {
		if(handler) (*handler)(ptr, "network_ipv6 syntax error #3");
		return false;
	}

	network.prefix = address;
	network.shift = shift;

	ptr = p;

	return true;
}

template<>
void out_t::helper_t<network_ipv6_t>::print(
	out_t &out, network_ipv6_t const &network, char const *fmt
) {
	bool raw = (fmt && *fmt == 'r');

	out.print(network.prefix, fmt);

	if(!raw && ipv6_ipv4_map.match(network.prefix))
		out('/').print(32 - network.shift);
	else
		out('/').print(128 - network.shift);
}

namespace config {

template<>
void helper_t<address_ipv6_t>::parse(in_t::ptr_t &ptr, address_ipv6_t &address) {
	ptr.parse(address, error);
}

template<>
void helper_t<address_ipv6_t>::print(
	out_t &out, int, address_ipv6_t const &address
) {
	out.print(address);
}

template<>
void helper_t<address_ipv6_t>::syntax(out_t &out) {
	out(CSTR("<ipv6_address>"));
}

template<>
void helper_t<network_ipv6_t>::parse(in_t::ptr_t &ptr, network_ipv6_t &network) {
	ptr.parse(network, error);
}

template<>
void helper_t<network_ipv6_t>::print(
	out_t &out, int, network_ipv6_t const &network
) {
	out.print(network);
}

template<>
void helper_t<network_ipv6_t>::syntax(out_t &out) {
	out(CSTR("<ipv6_network>"));
}

} // namespace config

} // namespace pd
