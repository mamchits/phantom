// This file is part of the phantom::io_stream::proto_monitor module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../proto.H"
#include "../acl.H"
#include "../../module.H"
#include "../../obj.H"

#include <pd/base/config.H>
#include <pd/base/config_list.H>
#include <pd/base/config_enum.H>

namespace phantom { namespace io_stream {

MODULE(io_stream_proto_monitor);

class proto_monitor_t : public proto_t {
	acl_t *acl;
	bool clear;
	config::list_t<obj_t *> const &list;

	virtual bool request_proc(
		in_t::ptr_t &ptr, out_t &out, netaddr_t const &, netaddr_t const &
	);

	virtual void stat(out_t &out, bool clear);

public:
	struct config_t {
		config::objptr_t<acl_t> acl;
		config::enum_t<bool> clear;
		config::list_t<obj_t *> list;

		inline config_t() throw() : acl(), clear(false), list() { }
		inline ~config_t() throw() { }
		inline void check(in_t::ptr_t const &) const { }
	};

	inline proto_monitor_t(string_t const &, config_t const &config) throw() :
		proto_t(), acl(config.acl), clear(config.clear),
		list(config.list) { }

	inline ~proto_monitor_t() throw() { }
};

static inline bool isspace(char c) {
	switch(c) {
		case ' ': case '\t': case '\r': case '\n': return true;
	}
	return false;
}

bool proto_monitor_t::request_proc(
	in_t::ptr_t &, out_t &out, netaddr_t const &, netaddr_t const &remote_addr
) {
	if(!acl || acl->check(remote_addr)) {
		for(typeof(list.ptr()) ptr = list; ptr; ++ptr) {
			obj_t *obj = ptr.val();
			out.lf()(CSTR("** "))(obj->name).lf();
			obj->stat(out, clear);
			out.lf();
		}
	}

	out.flush_all();

	return false;
}

void proto_monitor_t::stat(out_t &, bool) { }

namespace proto_monitor {
config_binding_sname(proto_monitor_t);
config_binding_value(proto_monitor_t, acl);
config_binding_value(proto_monitor_t, clear);
config_binding_value(proto_monitor_t, list);
config_binding_ctor(proto_t, proto_monitor_t);
}

}} // namespace phantom::io_stream
