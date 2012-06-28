// This file is part of the phantom::io_stream::proto_echo module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../proto.H"
#include "../../module.H"

#include <pd/base/config.H>

namespace phantom { namespace io_stream {

MODULE(io_stream_proto_echo);

class proto_echo_t : public proto_t {
	virtual bool request_proc(
		in_t::ptr_t &ptr, out_t &out, netaddr_t const &, netaddr_t const &
	);

	virtual void stat(out_t &out, bool clear);

public:
	struct config_t {
		inline config_t() throw() { }
		inline ~config_t() throw() { }
		inline void check(in_t::ptr_t const &) const { }
	};

	inline proto_echo_t(string_t const &, config_t const &) throw() :
		proto_t() { }

	inline ~proto_echo_t() throw() { }
};

bool proto_echo_t::request_proc(
	in_t::ptr_t &ptr, out_t &out, netaddr_t const &, netaddr_t const &
) {
	if(!ptr) return false;

	while(ptr.pending()) {
		str_t str = ptr.__chunk();
		out(str);
		ptr += str.size();
	}

	return true;
}

void proto_echo_t::stat(out_t &, bool) { }

namespace proto_echo {
config_binding_sname(proto_echo_t);
config_binding_ctor(proto_t, proto_echo_t);
}

}} // namespace phantom::io_stream
