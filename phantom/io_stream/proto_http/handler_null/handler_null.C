// This file is part of the phantom::io_stream::proto_http::handler_null module.
// Copyright (C) 2010-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../handler.H"

#include "../../../module.H"

namespace phantom { namespace io_stream { namespace proto_http {

MODULE(io_stream_proto_http_handler_null);

class handler_null_t : public handler_t {
	virtual void do_proc(request_t const &request, reply_t &reply) const;

	class content_t : public reply_t::content_t {
		virtual http::code_t code() const throw() { return http::code_200; }
		virtual void print_header(out_t &, http::server_t const &) const { }
		virtual ssize_t size() const throw() { return 0; }
		virtual bool print(out_t &) const { return true; }
		virtual ~content_t() throw() { }
	public:
		inline content_t() { }
	};

public:
	struct config_t : handler_t::config_t {
		inline config_t() throw() : handler_t::config_t() { }
		inline void check(in_t::ptr_t const &ptr) const {
			handler_t::config_t::check(ptr);
		}
		inline ~config_t() throw() { }
	};

	inline handler_null_t(string_t const &name, config_t const &config) throw() :
		handler_t(name, config) { }

	inline ~handler_null_t() throw() { }
};

namespace handler_null {
config_binding_sname(handler_null_t);
config_binding_parent(handler_null_t, handler_t, 1);
config_binding_ctor(handler_t, handler_null_t);
} // namespace handler_null

void handler_null_t::do_proc(request_t const &, reply_t &reply) const {
	reply.set(new content_t);
}

}}} // namespace phantom::io_stream::proto_http
