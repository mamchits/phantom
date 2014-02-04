// This file is part of the phantom::io_stream::proto_http::handler_fcgi module.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../handler.H"

#include "../../../module.H"

#include "../../../io_client/proto_fcgi/proto_fcgi.H"

#include <pd/bq/bq_util.H>

#include <pd/base/size.H>

namespace phantom { namespace io_stream { namespace proto_http {

MODULE(io_stream_proto_http_handler_fcgi);

class handler_fcgi_t : public handler_t {
	virtual void do_proc(request_t const &request, reply_t &reply) const;

	io_client::proto_fcgi_t &client_proto;
	interval_t timeout;
	size_t retries;
	string_t root;

public:
	struct config_t : handler_t::config_t {
		config::objptr_t<io_client::proto_fcgi_t> client_proto;
		interval_t timeout;
		sizeval_t retries;
		string_t root;

		inline config_t() throw() :
			handler_t::config_t(),
			timeout(interval::second), retries(3), root() { }

		inline void check(in_t::ptr_t const &ptr) const {
			handler_t::config_t::check(ptr);

			if(!client_proto)
				config::error(ptr, "client_proto is required");

			if(!retries)
				config::error(ptr, "retries is zero");
		}

		inline ~config_t() throw() { }
	};

	inline handler_fcgi_t(string_t const &_name, config_t const &config) :
		handler_t(_name, config), client_proto(*config.client_proto),
		timeout(config.timeout), retries(config.retries), root(config.root) { }

	inline ~handler_fcgi_t() throw() { }
};

namespace handler_fcgi {
config_binding_sname(handler_fcgi_t);
config_binding_value(handler_fcgi_t, client_proto);
config_binding_value(handler_fcgi_t, timeout);
config_binding_value(handler_fcgi_t, retries);
config_binding_value(handler_fcgi_t, root);
config_binding_parent(handler_fcgi_t, handler_t);
config_binding_ctor(handler_t, handler_fcgi_t);
}

void handler_fcgi_t::do_proc(request_t const &request, reply_t &reply) const {
	size_t i = 0;

	while(true) {
		interval_t att_timeout = timeout;

		if(client_proto.proc(request, reply, &att_timeout, root))
			return;

		if(++i == retries)
			throw http::exception_t(http::code_504, "Gateway Time-out");

		if(att_timeout > interval::zero)
			bq_sleep(&att_timeout);
	}
}

}}} // namespace phantom::io_stream::proto_http
