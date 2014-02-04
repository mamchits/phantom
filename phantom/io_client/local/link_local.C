// This file is part of the phantom::io_client::local module.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../link.H"
#include "../../module.H"

#include <pd/base/netaddr_local.H>
#include <pd/base/config.H>
#include <pd/base/config_list.H>
#include <pd/base/config_enum.H>

namespace phantom { namespace io_client {

MODULE(io_client_local);

class link_local_t : public link_t {
	netaddr_local_t netaddr;

	virtual fd_ctl_t const *ctl() const throw();
	virtual netaddr_t const &remote_netaddr() const throw();
	virtual ~link_local_t() throw();

public:
	inline link_local_t(
		unsigned int rank, interval_t _conn_timeout, log::level_t _remote_errors, proto_t &_proto,
		netaddr_local_t const &_netaddr
	) :
		link_t(rank, _conn_timeout, _remote_errors, _proto),
		netaddr(_netaddr) { }
};

fd_ctl_t const *link_local_t::ctl() const throw() { return NULL; }

netaddr_t const &link_local_t::remote_netaddr() const throw() { return netaddr; }

link_local_t::~link_local_t() throw() { }

class links_local_t : public links_t {
	sarray1_t<netaddr_local_t> addrs;

	virtual void create(
		pool_t &pool,
		interval_t _conn_timeout, log::level_t _remote_errors, proto_t &_proto
	) const;

public:
	struct config_t : links_t::config_t {
		config::list_t<string_t> paths;

		inline config_t() throw() :
			links_t::config_t(), paths() { }

		inline void check(in_t::ptr_t const &ptr) const {
			links_t::config_t::check(ptr);

			for(typeof(paths._ptr()) lptr = paths; lptr; ++lptr)
				if(lptr.val().size() > netaddr_local_t::max_len())
					config::error(ptr, "path is too long");
		}

		inline ~config_t() throw() { }
	};

	inline links_local_t(string_t const &name, config_t const &config) :
		links_t(name, config),
		addrs(
			config.paths,
			[](string_t const &string) { return netaddr_local_t(string.str()); }
		) { }

	inline ~links_local_t() throw() { }
};

void links_local_t::create(
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
		netaddr_local_t const &addr = addrs[i];

		bucket.setup(addr, count);

		for(size_t j = 0; j < count; ++j) {
			bucket[j].setup(
				stags[j],
				new link_local_t(
					rank, _conn_timeout, _remote_errors, _proto, addr
				)
			);
		}
	}
}

namespace links_local {
config_binding_sname(links_local_t);
config_binding_value(links_local_t, paths);
config_binding_parent(links_local_t, links_t);
config_binding_ctor(links_t, links_local_t);
}

}} // namespace phantom::io_client
