// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/netaddr.H>
#include <pd/base/ipv6.H>
#include <pd/base/assert.H>

#include <netinet/in.h>

#pragma GCC visibility push(default)

namespace pd {

class netaddr_ipv6_t : public netaddr_t {
	sockaddr_in6 sin6;

public:
	inline netaddr_ipv6_t() throw() : netaddr_t(sizeof(sin6)) {
		assert((sockaddr *)&sin6 == sa);
		sin6.sin6_family = AF_UNSPEC;
	}

	inline netaddr_ipv6_t(address_ipv6_t const &addr, uint16_t port) throw() :
		netaddr_t(sizeof(sin6)) {

		sin6.sin6_family = AF_INET6;
		sin6.sin6_addr.s6_addr32[0] = htonl((addr.value >> 96) & 0xffffffff);
		sin6.sin6_addr.s6_addr32[1] = htonl((addr.value >> 64) & 0xffffffff);
		sin6.sin6_addr.s6_addr32[2] = htonl((addr.value >> 32) & 0xffffffff);
		sin6.sin6_addr.s6_addr32[3] = htonl(addr.value & 0xffffffff);
		sin6.sin6_port = htons(port);
	}

	virtual void print(out_t &out, char const *fmt = NULL) const;
	virtual size_t print_len() const;

	virtual ~netaddr_ipv6_t() throw();

	inline address_ipv6_t address() const throw() {
		return address_ipv6_t(
			((uint128_t)ntohl(sin6.sin6_addr.s6_addr32[0]) << 96) |
			((uint128_t)ntohl(sin6.sin6_addr.s6_addr32[1]) << 64) |
			((uint128_t)ntohl(sin6.sin6_addr.s6_addr32[2]) << 32) |
			((uint128_t)ntohl(sin6.sin6_addr.s6_addr32[3]))
		);
	}

	inline uint16_t port() const throw() {
		return ntohs(sin6.sin6_port);
	}
};

} // namespace pd

#pragma GCC visibility pop
