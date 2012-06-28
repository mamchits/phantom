// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/netaddr.H>
#include <pd/base/ipv4.H>
#include <pd/base/assert.H>

#include <netinet/in.h>

#pragma GCC visibility push(default)

namespace pd {

class netaddr_ipv4_t : public netaddr_t {
	sockaddr_in sin;

public:
	inline netaddr_ipv4_t() throw() : netaddr_t(sizeof(sin)) {
		assert((sockaddr *)&sin == sa);
		sin.sin_family = AF_UNSPEC;
	}

	inline netaddr_ipv4_t(address_ipv4_t const &addr, uint16_t port) throw() :
		netaddr_t(sizeof(sin)) {

		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = htonl(addr.value);
		sin.sin_port = htons(port);
	}

	virtual void print(out_t &out, char const *fmt = NULL) const;
	virtual size_t print_len() const;

	virtual ~netaddr_ipv4_t() throw();

	inline address_ipv4_t address() const throw() {
		return address_ipv4_t(ntohl(sin.sin_addr.s_addr));
	}

	inline uint16_t port() const throw() {
		return ntohs(sin.sin_port);
	}
};

} // namespace pd

#pragma GCC visibility pop
