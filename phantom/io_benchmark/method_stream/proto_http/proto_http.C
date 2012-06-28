// This file is part of the phantom::io_benchmark::method_stream::proto_http module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
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

	virtual bool reply_parse(
		in_t::ptr_t &ptr, in_segment_t const &request,
		unsigned int &res_code, stat_t &stat, logger_t::level_t &lev
	) const;

	virtual size_t maxi() const throw();
	virtual descr_t const *descr(size_t ind) const throw();

public:
	struct config_t {
		config::struct_t<http::limits_t::config_t> reply_limits;

		inline config_t() throw() :
			reply_limits(1024, 128, 8 * sizeval_kilo, 8 * sizeval_mega) { }

		inline ~config_t() throw() { }
		inline void check(in_t::ptr_t const &) const { }
	};

	inline proto_http_t(
		string_t const &, config_t const &config
	) throw() : proto_t(), reply_limits(config.reply_limits) { }

	inline ~proto_http_t() throw() { }
};

namespace proto_http {
config_binding_sname(proto_http_t);
config_binding_value(proto_http_t, reply_limits);
config_binding_ctor(proto_t, proto_http_t);
}

static inline bool is_head_request(in_segment_t const &request) {
	in_t::ptr_t ptr = request;
	return ptr.match<lower_t>(CSTR("HEAD "));
}

bool proto_http_t::reply_parse(
	in_t::ptr_t &ptr, in_segment_t const &request,
	unsigned int &res_code, stat_t &stat, logger_t::level_t &lev
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

	{
		thr::spinlock_guard_t guard(stat.spinlock);
		stat.update(0, reply.code);
	}

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

class http_descr_t : public descr_t {
	static size_t const max_http_code = 600;

	virtual size_t value_max() const { return max_http_code; }

	virtual void print_header(out_t &out) const {
		out(CSTR("HTTP"));
	}

	virtual void print_value(out_t &out, size_t value) const {
		if(value < max_http_code) {
			switch(value) {
#define HTTP_CODE(code, string) \
				case code: out(CSTR(string)); break;

				HTTP_CODE_LIST

#undef HTTP_CODE
				default:
					out(CSTR("http code ")).print(value);
			}
		}
		else
			out(CSTR("Unknown http code"));
	}
public:
	inline http_descr_t() throw() : descr_t() { }
	inline ~http_descr_t() throw() { }
};

static http_descr_t const http_descr;

size_t proto_http_t::maxi() const throw() {
	return 1;
}

descr_t const *proto_http_t::descr(size_t ind) const throw() {
	if(ind == 0) return &http_descr;
	return NULL;
}

}}} // namespace phantom::io_benchmark::method_stream
