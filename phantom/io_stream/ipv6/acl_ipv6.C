// This file is part of the phantom::io_stream::ipv6 module.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../acl.H"

#include <pd/base/config.H>
#include <pd/base/config_list.H>
#include <pd/base/config_enum.H>
#include <pd/base/config_record.H>
#include <pd/base/netaddr_ipv6.H>

namespace phantom { namespace io_stream {

class acl_ipv6_t : public acl_t {
public:
	struct item_config_t {
		config::enum_t<policy_t> policy;
		network_ipv6_t network;

		inline item_config_t() throw() : policy(unset), network() { }
		inline ~item_config_t() throw() { }
	};

private:
	struct item_t {
		network_ipv6_t network;
		bool flag;

		inline item_t() throw() : network(), flag(true) { }
		inline ~item_t() throw() { }

		inline item_t &operator=(item_config_t const &config) {
			network = config.network;
			flag = (config.policy == allow);
			return *this;
		}
	};

	struct items_t : sarray1_t<item_t> {
		inline items_t(config::list_t<config::record_t<item_config_t>> const &list) :
			sarray1_t<item_t>(list) { }

		inline ~items_t() throw() { }

		inline bool check(netaddr_t const &netaddr, bool def_flag) const {
			if(netaddr.sa->sa_family != AF_INET6)
				return def_flag;

			address_ipv6_t addr = ((netaddr_ipv6_t const &)netaddr).address();

			for(size_t i = 0; i < size; ++i) {
				item_t const &item = items[i];

				if(item.network.match(addr))
					return item.flag;
			}

			return def_flag;
		}
	};

	virtual bool check(netaddr_t const &netaddr) const throw();

	bool def_flag;
	items_t items;

public:
	struct config_t {
		config::list_t<config::record_t<item_config_t>> list;
		config::enum_t<policy_t> default_policy;

		inline config_t() throw() : list(), default_policy(unset) { }
		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &ptr) const {
			if(!default_policy)
				config::error(ptr, "default_policy is required");
		}
	};

	inline acl_ipv6_t(string_t const &, config_t const &config) :
		def_flag(config.default_policy == allow),
		items(config.list) { }

	inline ~acl_ipv6_t() throw() { }
};

namespace acl_ipv6 {
config_binding_sname(acl_ipv6_t);
config_binding_value(acl_ipv6_t, default_policy);
config_binding_value(acl_ipv6_t, list);
config_binding_cast(acl_ipv6_t, acl_t);
config_binding_ctor(acl_t, acl_ipv6_t);

namespace item {
config_record_sname(acl_ipv6_t::item_config_t);
config_record_value(acl_ipv6_t::item_config_t, policy);
config_record_value(acl_ipv6_t::item_config_t, network);
}}

bool acl_ipv6_t::check(netaddr_t const &netaddr) const throw() {
	return items.check(netaddr, def_flag);
}

}} // namespace phantom::io_stream_ipv6
