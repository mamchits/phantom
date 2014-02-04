// This file is part of the phantom::io_stream::proto_http::handler_proxy module.
// Copyright (C) 2010-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../handler.H"

#include "../../../module.H"

#include "../../../io_client/proto_none/proto_none.H"
#include "../../../io_client/proto_none/task.H"

#include <pd/http/limits_config.H>
#include <pd/http/client.H>

#include <pd/base/exception.H>
#include <pd/base/config_struct.H>

namespace phantom { namespace io_stream { namespace proto_http {

MODULE(io_stream_proto_http_handler_proxy);

class handler_proxy_t : public handler_t {
	virtual void do_proc(request_t const &request, reply_t &reply) const;

	class task_t;
	class content_t;

	io_client::proto_none_t &client_proto;
	interval_t timeout;
	http::limits_t reply_limits;

public:
	struct config_t : handler_t::config_t {
		config::objptr_t<io_client::proto_none_t> client_proto;
		interval_t timeout;
		config::struct_t<http::limits_t::config_t> reply_limits;

		inline config_t() throw() :
			handler_t::config_t(),
			client_proto(), timeout(interval::second),
			reply_limits(1024, 128, 8 * sizeval::kilo, 8 * sizeval::mega) { }

		inline void check(in_t::ptr_t const &ptr) const {
			handler_t::config_t::check(ptr);

			if(!client_proto)
				config::error(ptr, "client_proto is required");
		}

		inline ~config_t() throw() { }
	};

	inline handler_proxy_t(string_t const &name, config_t const &config) :
		handler_t(name, config),
		client_proto(*config.client_proto), timeout(config.timeout),
		reply_limits(config.reply_limits) { }

	inline ~handler_proxy_t() throw() { }
};

namespace handler_proxy {
config_binding_sname(handler_proxy_t);
config_binding_value(handler_proxy_t, client_proto);
config_binding_value(handler_proxy_t, timeout);
config_binding_value(handler_proxy_t, reply_limits);
config_binding_parent(handler_proxy_t, handler_t);
config_binding_ctor(handler_t, handler_proxy_t);
} // handler_proxy

class handler_proxy_t::task_t : public io_client::proto_none::task_t {
	http::method_t method;
	in_segment_t uri_path;
	in_segment_t uri_args;

	struct header_t {
		struct item_t {
			in_segment_t key;
			in_segment_t val;

			inline item_t() : key(), val() { }

			inline void setup(in_segment_t const &_key, in_segment_t const &_val) {
				key = _key;
				val = _val;
			}

			inline void print(out_t &out) const {
				out(key)(':')(val); // ' ' and CRLF inside val
			}

			inline ~item_t() throw() { }
		};

		size_t size, maxsize;
		item_t *items;

		inline header_t(size_t _maxsize) :
			size(0), maxsize(_maxsize), items(new item_t[_maxsize]) { }

		inline void add(in_segment_t const &_key, in_segment_t const &_val) {
			assert(size < maxsize);
			items[size++].setup(_key, _val);
		}

		inline void print(out_t &out) const {
			for(size_t i = 0; i < size; ++i) {
				items[i].print(out);
			}
		}

		inline ~header_t() { delete [] items; }
	};

	header_t header;
	in_segment_t entity;

	string_t host;
	string_t remote_addr;
	bool need_xrip, need_host;

	http::limits_t const &reply_limits;
	http::remote_reply_t reply;

	virtual void clear() {
		reply.clear();
	}

	virtual bool parse(in_t::ptr_t &ptr) {
		try {
			return reply.parse(ptr, reply_limits, method == http::method_head);
		}
		catch(http::exception_t const &ex) {
			str_t msg = ex.msg();
			throw exception_sys_t(log::error, EPROTO, "%.*s", (int)msg.size(), msg.ptr());
		}
	}

	virtual void print(out_t &out) const {
		switch(method) {
			case http::method_head: out(CSTR("HEAD ")); break;
			case http::method_get: out(CSTR("GET ")); break;
			case http::method_post: out(CSTR("POST ")); break;
			case http::method_options: out(CSTR("OPTIONS ")); break;
			case http::method_undefined: out(CSTR("UNDEFINED ")); break;
		}
		out(uri_path);
		if(uri_args) out('?')(uri_args);
		out(CSTR(" HTTP/1.1")).crlf();

		header.print(out);
		if(need_host)
			out(CSTR("Host: "))(host).crlf();

		if(need_xrip)
			out(CSTR("X-Real-IP: "))(remote_addr).crlf();

		out(CSTR("Connection: keep-alive")).crlf();

		out(CSTR("Content-Length: ")).print(entity.size()).crlf();
		out.crlf();

		out(entity);
	}

	virtual ~task_t() throw() { }

public:
	inline task_t(
		http::remote_request_t const &request, http::limits_t const &_reply_limits
	) :
		method(request.method), uri_path(request.uri_path),
		uri_args(request.uri_args),
		header(request.header.size() + request.entity.trailer.size()),
		entity(request.entity),
		host(request.host), remote_addr(
			string_t::ctor_t(request.remote_addr.print_len()).print(request.remote_addr)
		),
		need_xrip(true), need_host(true),
		reply_limits(_reply_limits), reply() {

		size_t hsize = request.header.size();

		for(size_t i = 0; i < hsize; ++i) {
			in_segment_t const &key = request.header.key(i);

			if(key.cmp_eq<lower_t>(CSTR("connection")))
				continue;
			else if(key.cmp_eq<lower_t>(CSTR("transfer-encoding")))
				continue;
			else if(key.cmp_eq<lower_t>(CSTR("content-length")))
				continue;
			else if(key.cmp_eq<lower_t>(CSTR("host")))
				need_host = false;
			else if(key.cmp_eq<lower_t>(CSTR("x-real-ip")))
				need_xrip = false;

			header.add(key, request.header.val(i));
		}

		hsize = request.entity.trailer.size();

		for(size_t i = 0; i < hsize; ++i) {
			header.add(request.entity.trailer.key(i), request.entity.trailer.val(i));
		}
	}

	friend class content_t;
};

class handler_proxy_t::content_t : public reply_t::content_t {
	task_t *task;
	ref_t<io_client::proto_none::task_t> task_ref;

	virtual http::code_t code() const throw() {
		return task->reply.code;
	}

	virtual void print_header(out_t &out, http::server_t const &server) const {
		http::mime_header_t const &reply_header = task->reply.header;

		size_t hsize = reply_header.size();
		for(size_t i = 0; i < hsize; ++i) {
			in_segment_t const &key = reply_header.key(i);

			if(key.cmp_eq<lower_t>(CSTR("connection")))
				continue;
			else if(key.cmp_eq<lower_t>(CSTR("transfer-encoding")))
				continue;
			else if(key.cmp_eq<lower_t>(CSTR("content-length")))
				continue;
			else if(key.cmp_eq<lower_t>(CSTR("date")))
				continue;
			else if(!server.filter_header(key))
				continue;

			out(key)(':')(reply_header.val(i)); // ' ' and CRLF inside val
		}
	}

	virtual ssize_t size() const {
		if(task->method != http::method_head)
			return task->reply.entity.size();

		in_segment_t const *val = task->reply.header.lookup(CSTR("content-length"));
		size_t len = 0;

		if(val)
			http::number_parse(*val, len);

		return len;
	}

	virtual bool print(out_t &out) const {
		if(task->method != http::method_head)
			out(task->reply.entity);

		return true;
	}

	virtual ~content_t() throw() { }

public:
	inline content_t(task_t *_task) throw() : task(_task), task_ref(_task) { }
};

void handler_proxy_t::do_proc(request_t const &request, reply_t &reply) const {
	task_t *task = new task_t(request, reply_limits);

	ref_t<io_client::proto_none::task_t> task_ref(task);

	client_proto.put_task(task_ref);
	interval_t s = timeout;
	if(!task_ref->wait(&s, NULL)) {
		task_ref->cancel();
		throw http::exception_t(http::code_504, "Gateway Time-out");
	}

	reply.set(new content_t(task));
}

}}} // namespace phantom::io_stream::proto_http
