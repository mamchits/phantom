// This file is part of the phantom::io_benchmark::method_stream::proto_none module.
// Copyright (C) 2009-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2009-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../proto.H"

#include "../../../module.H"

#include <pd/base/config.H>
#include <pd/base/exception.H>

namespace phantom { namespace io_benchmark { namespace method_stream {

MODULE(io_benchmark_method_stream_proto_none);

class proto_none_t : public proto_t {
	virtual void do_init() { }
	virtual void do_run() const { }
	virtual void do_stat_print() const { }
	virtual void do_fini() { }

	virtual bool reply_parse(
		in_t::ptr_t &ptr, in_segment_t const &request,
		unsigned int &res_code, logger_t::level_t &lev
	) const;

public:
	struct config_t {
		inline config_t() throw() { }
		inline ~config_t() throw() { }
		inline void check(in_t::ptr_t const &) const { }
	};

	inline proto_none_t(string_t const &name, config_t const &) throw() :
		proto_t(name) { }

	inline ~proto_none_t() throw() { }
};

namespace proto_none {
config_binding_sname(proto_none_t);
config_binding_cast(proto_none_t, proto_t);
config_binding_ctor(proto_t, proto_none_t);
}

bool proto_none_t::reply_parse(
	in_t::ptr_t &ptr, in_segment_t const &,
	unsigned int &, logger_t::level_t &lev
) const {
	while(ptr)
		ptr.seek_end();

	lev = logger_t::all;

	return false;
}

}}} // namespace phantom::io_benchmark::method_stream
