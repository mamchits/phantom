// This file is part of the phantom::io_benchmark::method_stream::proto_http module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../proto.H"

#include "../../../module.H"

#include <pd/http/limits_config.H>
#include <pd/http/client.H>

#include <pd/base/config_struct.H>
#include <pd/base/exception.H>

namespace phantom { namespace io_benchmark { namespace method_stream {

MODULE(io_benchmark_method_stream_proto_http);

class proto_http_t : public proto_t {
	http::limits_t reply_limits;

	static mcount_t::tags_t const &tags;

	mcount_t mutable mcount;

	virtual void do_init() { mcount.init(); }
	virtual void do_run() const { }
	virtual void do_stat_print() const { mcount.print(); }
	virtual void do_fini() { }

	virtual bool reply_parse(
		in_t::ptr_t &ptr, in_segment_t const &request,
		unsigned int &res_code, logger_t::level_t &lev
	) const;

public:
	struct config_t {
		config::struct_t<http::limits_t::config_t> reply_limits;

		inline config_t() throw() :
			reply_limits(1024, 128, 8 * sizeval::kilo, 8 * sizeval::mega) { }

		inline ~config_t() throw() { }
		inline void check(in_t::ptr_t const &) const { }
	};

	inline proto_http_t(string_t const &name, config_t const &config) throw() :
		proto_t(name), reply_limits(config.reply_limits), mcount(tags) { }

	inline ~proto_http_t() throw() { }
};

namespace proto_http {
config_binding_sname(proto_http_t);
config_binding_value(proto_http_t, reply_limits);
config_binding_cast(proto_http_t, proto_t);
config_binding_ctor(proto_t, proto_http_t);
}

namespace proto_http {

static string_t const _tags[] {
	STRING("Undefined"),
#define HTTP_CODE(x, y) \
	STRING(y),

	HTTP_CODE_LIST

#undef HTTP_CODE
	STRING("Other")
};

static size_t const _tags_count = sizeof(_tags) / sizeof(_tags[0]);

static size_t code_idx(http::code_t code) {
	size_t idx = 0;
	switch(code) {
		case http::code_undefined: ++idx;

#define HTTP_CODE(x, y) \
		case http::code_##x: ++idx;

		HTTP_CODE_LIST

#undef HTTP_CODE
	}

	return _tags_count - 1 - idx;
}

struct tags_t : mcount_t::tags_t {
	inline tags_t() : mcount_t::tags_t() { }
	inline ~tags_t() throw() { }

	virtual size_t size() const { return _tags_count; }

	virtual void print(out_t &out, size_t idx) const {
		assert(idx < _tags_count);

		out(_tags[idx]);
	}
};

tags_t const tags;

} // namespace proto_http

mcount_t::tags_t const &proto_http_t::tags = proto_http::tags;

static inline bool is_head_request(in_segment_t const &request) {
	in_t::ptr_t ptr = request;
	return ptr.match<lower_t>(CSTR("HEAD "));
}

bool proto_http_t::reply_parse(
	in_t::ptr_t &ptr, in_segment_t const &request,
	unsigned int &res_code, logger_t::level_t &lev
) const {
	http::remote_reply_t reply;
	bool force_close = false;

	try {
		if(!reply.parse(ptr, reply_limits, is_head_request(request)))
			force_close = true;
	}
	catch(http::exception_t const &ex) {
		str_t msg = ex.msg();
		throw exception_sys_t(log::error, EPROTO, "%.*s", (int)msg.size(), msg.ptr());
	}

	res_code = reply.code;

	mcount.inc(proto_http::code_idx(reply.code));

	if(reply.code >= 200 && reply.code < 400)
		lev = logger_t::all;
	else if(reply.code >= 400 && reply.code < 500)
		lev = logger_t::proto_warning;
	else
		lev = logger_t::proto_error;

	return (!force_close && ({
		bool keepalive = (reply.version >= http::version_1_1);

		in_segment_t const *val = reply.header.lookup(CSTR("connection"));

		if(val) {
			if(http::token_find(*val, CSTR("close")))
				keepalive = false;
			else if(http::token_find(*val, CSTR("keep-alive")))
				keepalive = true;
		}

		keepalive;
	}));
}

}}} // namespace phantom::io_benchmark::method_stream
