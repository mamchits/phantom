// This file is part of the phantom::io_stream::ipv4 module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../io_stream.H"
#include "../../module.H"

#include <pd/base/netaddr_ipv4.H>
#include <pd/base/fd_tcp.H>
#include <pd/base/exception.H>

#include <netinet/tcp.h>

namespace phantom {

MODULE(io_stream_ipv4);

class io_stream_ipv4_t : public io_stream_t {
	netaddr_ipv4_t netaddr;
	bool defer_accept;
	fd_ctl_tcp_t ctl_tcp;

	virtual netaddr_t const &bind_addr() const throw();
	virtual netaddr_t *new_netaddr() const;
	virtual void fd_setup(int fd) const;
	virtual fd_ctl_t const *ctl() const;

public:
	struct config_t : io_stream_t::config_t {
		address_ipv4_t address;
		uint16_t port;
		config::enum_t<bool> defer_accept, cork;

		inline config_t() throw() :
			io_stream_t::config_t(), address(), port(0),
			defer_accept(false), cork(true) { }

		inline void check(in_t::ptr_t const &ptr) const {
			io_stream_t::config_t::check(ptr);

			if(!port)
				config::error(ptr, "port is required");
		}

		inline ~config_t() throw() { }
	};

	inline io_stream_ipv4_t(string_t const &name, config_t const &config) :
		io_stream_t(name, config), netaddr(config.address, config.port),
		defer_accept(config.defer_accept), ctl_tcp(config.cork) { }

	inline ~io_stream_ipv4_t() throw() { }
};

netaddr_t const &io_stream_ipv4_t::bind_addr() const throw() {
	return netaddr;
}

netaddr_t *io_stream_ipv4_t::new_netaddr() const {
	return new netaddr_ipv4_t;
}

void io_stream_ipv4_t::fd_setup(int fd) const {
	if(defer_accept) {
		int i = 1;
		if(setsockopt(fd, SOL_TCP, TCP_DEFER_ACCEPT, &i, sizeof(i)) < 0)
			throw exception_sys_t(log::error, errno, "setsockopt, TCP_DEFER_ACCEPT, 1: %m");
	}
}

fd_ctl_t const *io_stream_ipv4_t::ctl() const {
	return &ctl_tcp;
}

namespace io_stream_ipv4 {
config_binding_sname(io_stream_ipv4_t);
config_binding_value(io_stream_ipv4_t, address);
config_binding_value(io_stream_ipv4_t, port);
config_binding_parent(io_stream_ipv4_t, io_stream_t, 1);
config_binding_value(io_stream_ipv4_t, defer_accept);
config_binding_value(io_stream_ipv4_t, cork);
config_binding_ctor(io_t, io_stream_ipv4_t);
}

} // namespace phantom
