// This file is part of the phantom::io_client::ipv4 module.
// Copyright (C) 2010-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../link.H"
#include "../../module.H"

#include <pd/base/netaddr_ipv4.H>
#include <pd/base/fd_tcp.H>
#include <pd/base/config.H>
#include <pd/base/config_list.H>
#include <pd/base/config_enum.H>

namespace phantom { namespace io_client {

MODULE(io_client_ipv4);

class link_ipv4_t : public link_t {
	netaddr_ipv4_t netaddr;
	fd_ctl_tcp_t ctl_tcp;

	virtual fd_ctl_t const *ctl() const throw();
	virtual netaddr_t const &remote_netaddr() const throw();
	virtual ~link_ipv4_t() throw();

public:
	inline link_ipv4_t(
		unsigned int rank, interval_t _conn_timeout,
		log::level_t _remote_errors, proto_t &_proto,
		netaddr_ipv4_t const &_netaddr, bool cork
	) :
		link_t(rank, _conn_timeout, _remote_errors, _proto),
		netaddr(_netaddr), ctl_tcp(cork) { }
};

fd_ctl_t const *link_ipv4_t::ctl() const throw() { return &ctl_tcp; }

netaddr_t const &link_ipv4_t::remote_netaddr() const throw() { return netaddr; }

link_ipv4_t::~link_ipv4_t() throw() { }

class links_ipv4_t : public links_t {
	sarray1_t<netaddr_ipv4_t> addrs;
	bool cork;

	virtual void create(
		pool_t &pool,
		interval_t _conn_timeout, log::level_t _remote_errors, proto_t &_proto
	) const;

public:
	struct config_t : links_t::config_t {
		config::list_t<address_ipv4_t> addresses;
		uint16_t port;
		config::enum_t<bool> cork;

		inline config_t() throw() :
			links_t::config_t(), addresses(), port(0), cork(true) { }

		inline void check(in_t::ptr_t const &ptr) const {
			links_t::config_t::check(ptr);

			if(!port)
				config::error(ptr, "port is required");
		}

		inline ~config_t() throw() { }
	};

	inline links_ipv4_t(string_t const &name, config_t const &config) :
		links_t(name, config),
		addrs(
			config.addresses,
			[&config](address_ipv4_t const &addr) {
				return netaddr_ipv4_t(addr, config.port);
			}
		),
		cork(config.cork) { }

	inline ~links_ipv4_t() throw() { }
};

void links_ipv4_t::create(
	pool_t &pool,
	interval_t _conn_timeout, log::level_t _remote_errors, proto_t &_proto
) const {
	char const *fmt = log::number_fmt(count);

	string_t stags[count];

	for(size_t j = 0; j < count; ++j)
		stags[j] = string_t::ctor_t(5).print(j, fmt);

	pool.setup(name, addrs.size);

	for(size_t i = 0; i < addrs.size; ++i) {
		pool_t::bucket_t &bucket = pool[i];
		netaddr_ipv4_t const &addr = addrs[i];

		bucket.setup(addr, count);

		for(size_t j = 0; j < count; ++j)
			bucket[j].setup(
				stags[j],
				new link_ipv4_t(
					rank, _conn_timeout, _remote_errors, _proto, addr, cork
				)
			);
	}
}

namespace links_ipv4 {
config_binding_sname(links_ipv4_t);
config_binding_value(links_ipv4_t, addresses);
config_binding_value(links_ipv4_t, port);
config_binding_value(links_ipv4_t, cork);
config_binding_parent(links_ipv4_t, links_t);
config_binding_ctor(links_t, links_ipv4_t);
}

}} // namespace phantom::io_client
