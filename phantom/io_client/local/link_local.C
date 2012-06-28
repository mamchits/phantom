// This file is part of the phantom::io_client::local module.
// Copyright (C) 2011, 2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011, 2012, YANDEX LLC.
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
		link_t *&list, string_t const &name,
		interval_t _conn_timeout, proto_t &_proto,
		netaddr_local_t const &_netaddr
	) :
		link_t(list, name, _conn_timeout, _proto), netaddr(_netaddr) { }
};

fd_ctl_t const *link_local_t::ctl() const throw() { return NULL; }

netaddr_t const &link_local_t::remote_netaddr() const throw() { return netaddr; }

link_local_t::~link_local_t() throw() { }

class links_local_t : public links_t {
	size_t addrs_count;
	netaddr_local_t *addrs;

	virtual void create(
		link_t *&list, string_t const &client_name,
		interval_t _conn_timeout, proto_t &_proto
	) const;

public:
	struct config_t : links_t::config_t {

		config::list_t<string_t> paths;

		inline config_t() throw() :
			links_t::config_t(), paths() { }

		inline void check(in_t::ptr_t const &ptr) const {
			links_t::config_t::check(ptr);

			for(typeof(paths.ptr()) lptr = paths; lptr; ++lptr)
				if(lptr.val().size() > netaddr_local_t::max_len())
					config::error(ptr, "path is too long");
		}

		inline ~config_t() throw() { }
	};

	inline links_local_t(string_t const &name, config_t const &config) :
		links_t(name, config), addrs_count(0), addrs(NULL) {

		for(typeof(config.paths.ptr()) ptr = config.paths; ptr; ++ptr)
			++addrs_count;

		addrs = new netaddr_local_t[addrs_count];

		try {
			netaddr_local_t *aptr = addrs;

			for(typeof(config.paths.ptr()) ptr = config.paths; ptr; ++ptr)
				*(aptr++) = netaddr_local_t(ptr.val().str());
		}
		catch(...) {
			delete [] addrs;
			throw;
		}
	}

	inline ~links_local_t() throw() { if(addrs) delete [] addrs; }
};

void links_local_t::create(
	link_t *&list, string_t const &client_name,
	interval_t _conn_timeout, proto_t &_proto
) const {
	for(size_t i = 0; i < addrs_count; ++i) {
		netaddr_local_t const &addr = addrs[i];
		size_t addr_len = addr.print_len();

		for(size_t j = 0; j < count; ++j) {
			string_t link_name = string_t::ctor_t(
				client_name.size() + 1 + name.size() + 1 + addr_len + 1 + 6 + 1
			)(client_name)('(')(name)(',').print(addr)(',').print(j)(')');

			new link_local_t(list, link_name, _conn_timeout, _proto, addr);
		}
	}
}

namespace links_local {
config_binding_sname(links_local_t);
config_binding_value(links_local_t, paths);
config_binding_parent(links_local_t, links_t, 1);
config_binding_ctor(links_t, links_local_t);
}

}} // namespace phantom::io_client
