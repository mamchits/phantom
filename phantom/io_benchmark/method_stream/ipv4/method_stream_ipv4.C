// This file is part of the phantom::io_benchmark::method_stream::ipv4 module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../method_stream.H"

#include "../../../module.H"

#include <pd/bq/bq_util.H>

#include <pd/base/config_enum.H>
#include <pd/base/netaddr_ipv4.H>
#include <pd/base/fd_tcp.H>
#include <pd/base/exception.H>

#include <unistd.h>

namespace phantom { namespace io_benchmark {

MODULE(io_benchmark_method_stream_ipv4);

struct method_stream_ipv4_t : public method_stream_t {
	netaddr_ipv4_t netaddr;
	fd_ctl_tcp_t ctl_tcp;

	class locals_t {
		spinlock_t spinlock;
		size_t ind;
		sarray1_t<netaddr_ipv4_t> addrs;

	public:
		inline locals_t(config::list_t<address_ipv4_t> const &list) :
			spinlock(), ind(0),
			addrs(
				list,
				[](address_ipv4_t const &addr) { return netaddr_ipv4_t(addr, 0); }
			) { }

		inline ~locals_t() throw() { }

		inline void bind(int fd) {
			int i = 1;
			if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) < 0)
				throw exception_sys_t(log::error, errno, "setsockopt, SO_REUSEADDR, 1: %m");

			if(!addrs.size)
				return;

			size_t r = 0;

		_retry:
			netaddr_t const &addr = ({
				spinlock_guard_t guard(spinlock);
				if(ind == addrs.size) ind = 0;
				addrs[ind++];
			});

			if(::bind(fd, addr.sa, addr.sa_len) < 0) {
				if(errno == EADDRNOTAVAIL && ++r < addrs.size)
					goto _retry;

				throw exception_sys_t(log::error, errno, "bind: %m");
			}
		}
	};

	locals_t *locals;

	virtual netaddr_t const &dest_addr() const;
	virtual void bind(int fd) const;
	virtual fd_ctl_t const *ctl() const throw() { return &ctl_tcp; }

public:
	struct config_t : method_stream_t::config_t {
		address_ipv4_t address;
		uint16_t port;
		config::list_t<address_ipv4_t> bind;
		config::enum_t<bool> cork;

		inline config_t() throw() :
			method_stream_t::config_t(), address(), port(0), bind(), cork(true) { }

		inline void check(in_t::ptr_t const &ptr) const {
			method_stream_t::config_t::check(ptr);

			if(!address)
				config::error(ptr, "address is required");

			if(!port)
				config::error(ptr, "port is required");
		}
	};

	inline method_stream_ipv4_t(string_t const &name, config_t const &config) :
		method_stream_t(name, config), netaddr(config.address, config.port),
		ctl_tcp(config.cork), locals(NULL) {

		if(config.bind)
			locals = new locals_t(config.bind);
	}

	inline ~method_stream_ipv4_t() throw() {
		if(locals) delete locals;
	}
};

netaddr_t const &method_stream_ipv4_t::dest_addr() const {
	return netaddr;
}

void method_stream_ipv4_t::bind(int fd) const {
	if(locals)
		locals->bind(fd);
}

namespace method_stream_ipv4 {
config_binding_sname(method_stream_ipv4_t);
config_binding_value(method_stream_ipv4_t, address);
config_binding_value(method_stream_ipv4_t, port);
config_binding_value(method_stream_ipv4_t, bind);
config_binding_value(method_stream_ipv4_t, cork);
config_binding_parent(method_stream_ipv4_t, method_stream_t);
config_binding_cast(method_stream_ipv4_t, method_t);
config_binding_ctor(method_t, method_stream_ipv4_t);
}

}} // namespace phantom::io_benchmark
