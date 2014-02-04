// This file is part of the phantom::io_stream::proto_monitor module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../proto.H"
#include "../acl.H"
#include "../../module.H"
#include "../../obj.H"
#include "../../stat.H"

#include <pd/base/config.H>
#include <pd/base/config_list.H>
#include <pd/base/config_enum.H>

namespace phantom { namespace io_stream {

MODULE(io_stream_proto_monitor);

class proto_monitor_t : public proto_t {
	acl_t *acl;
	bool clear;
	size_t res_no;
	sarray1_t<obj_t const *> list;

	virtual void do_init() { }
	virtual void do_run() const { }
	virtual void do_stat_print() const { }
	virtual void do_fini() { }

	virtual bool request_proc(
		in_t::ptr_t &ptr, out_t &out, netaddr_t const &, netaddr_t const &
	) const;

public:
	struct config_t {
		config::objptr_t<acl_t> acl;
		config::enum_t<bool> clear;
		config::list_t<obj_t const *> list;
		stat_id_t stat_id;

		inline config_t() throw() : acl(), clear(false), list(), stat_id() { }
		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &ptr) const {
			if(!stat_id)
				config::error(ptr, "stat_id is required");
		}
	};

	inline proto_monitor_t(string_t const &_name, config_t const &config) throw() :
		proto_t(_name), acl(config.acl),
		clear(config.clear), res_no(config.stat_id.res_no), list(config.list) { }

	inline ~proto_monitor_t() throw() { }
};

bool proto_monitor_t::request_proc(
	in_t::ptr_t &, out_t &out, netaddr_t const &, netaddr_t const &remote_addr
) const {
	if(!acl || acl->check(remote_addr)) {
		stat::ctx_t ctx(out, stat::ctx_t::json, res_no, clear);
		obj_stat_print(list.items, list.size);
	}

	return false;
}

namespace proto_monitor {
config_binding_sname(proto_monitor_t);
config_binding_value(proto_monitor_t, acl);
config_binding_value(proto_monitor_t, clear);
config_binding_value(proto_monitor_t, list);
config_binding_value(proto_monitor_t, stat_id);
config_binding_cast(proto_monitor_t, proto_t);
config_binding_ctor(proto_t, proto_monitor_t);
}

}} // namespace phantom::io_stream
