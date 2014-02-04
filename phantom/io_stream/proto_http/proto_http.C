// This file is part of the phantom::io_stream::proto_http module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "handler.H"
#include "logger.H"
#include "path.H"

#include "../proto.H"
#include "../acl.H"
#include "../../module.H"

#include <pd/http/limits_config.H>

#include <pd/base/exception.H>
#include <pd/base/stat.H>
#include <pd/base/stat_items.H>
#include <pd/base/config_list.H>

namespace phantom { namespace io_stream {

MODULE(io_stream_proto_http);

namespace proto_http {

class p3p_t {
	string_t compact_value;
	string_t reference;

public:
	inline void print_header(out_t &out) const {
		if(compact_value) {
			if(reference)
				out(CSTR("P3P: policyref=\""))(reference)
					(CSTR("\", CP=\""))(compact_value)('\"').crlf();
			else
				out(CSTR("P3P: CP=\""))(compact_value)('\"').crlf();
		}
		else if(reference)
			out(CSTR("P3P: policyref=\""))(reference)('\"').crlf();
	}

	struct config_t {
		string_t compact_value;
		string_t reference;

		inline config_t() throw() : compact_value(), reference() { }
		inline void check(in_t::ptr_t const &) const { }
		inline ~config_t() throw() { }
	};

	inline p3p_t(string_t const &, config_t const &config) :
		compact_value(config.compact_value), reference(config.reference) { }

	inline ~p3p_t() throw() { }

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

namespace p3p {
config_binding_sname(p3p_t);
config_binding_value(p3p_t, compact_value);
config_binding_value(p3p_t, reference);
config_binding_ctor_(p3p_t);
}

struct server_t : http::server_t {
	p3p_t const *p3p;

	virtual void print_header(out_t &out) const;
	virtual bool filter_header(in_segment_t const &string) const;

public:
	inline server_t(p3p_t const *_p3p) throw() : p3p(_p3p) { }
	inline ~server_t() throw() { }
};

void server_t::print_header(out_t &out) const {
	out(CSTR("Server: Phantom/0.0.0")).crlf();
	if(p3p)
		p3p->print_header(out);
}

bool server_t::filter_header(in_segment_t const &key) const {
	if(key.cmp_eq<lower_t>(CSTR("server")))
		return false;

	if(p3p && key.cmp_eq<lower_t>(CSTR("p3p")))
		return false;

	return true;
}

} // namespace proto_http

class proto_http_t : public proto_t {
public:
	typedef http::remote_request_t request_t;
	typedef http::local_reply_t reply_t;

	typedef proto_http::handler_t handler_t;
	typedef proto_http::logger_t logger_t;
	typedef proto_http::path_t path_t;
	typedef proto_http::p3p_t p3p_t;
	typedef proto_http::server_t server_t;

	struct path_config_t {
		config::objptr_t<acl_t> acl;
		config::objptr_t<p3p_t> p3p;
		config::objptr_t<handler_t> handler;

		inline path_config_t() throw() : acl(), p3p(), handler() { }
		inline void check(in_t::ptr_t const &ptr) const {
			if(!handler)
				config::error(ptr, "handler is required");
		}
		inline ~path_config_t() throw() { }
	};

	typedef config::switch_t<path_t, config::struct_t<path_config_t>> path_switch_t;

private:
	struct path_data_t {
		typedef stat::count_t count_t;

		typedef stat::items_t<count_t> stat_base_t;

		struct stat_t : stat_base_t {
			inline stat_t() throw() : stat_base_t(STRING("hits")) { }

			inline ~stat_t() throw() { }

			inline count_t &count() throw() { return item<0>(); }
		};

		stat_t mutable stat;

		acl_t const *acl;
		p3p_t const *p3p;
		handler_t const *handler;

		inline path_data_t(path_config_t const &config) throw() :
			stat(), acl(config.acl), p3p(config.p3p), handler(config.handler) { }

		inline path_data_t() throw() :
			stat(), acl(NULL), p3p(NULL), handler(NULL) { }

		inline ~path_data_t() throw() { }

		path_data_t(path_data_t const &) = delete;
		path_data_t &operator=(path_data_t const &) = delete;

		inline void init() { stat.init(); }
		inline void stat_print() const { stat.print(); }
	};

	struct paths_t : proto_http::paths_t<path_data_t> {
		inline paths_t(path_switch_t const &path_switch) :
			proto_http::paths_t<path_data_t>(path_switch) { }

		inline ~paths_t() throw() { }

		inline void init() {
			for(size_t i = 0; i < size; ++i) {
				item_t *item = items[i];
				item->val.init();
			}
		}

		inline void stat_print() const {
			for(size_t i = 0; i < size; ++i) {
				item_t *item = items[i];
				str_t path = item->key.str();
				if(!path) path = CSTR("/");

				stat::ctx_t ctx(path);
				item->val.stat_print();
			}
		}
	};

public:
	class host_t : public string_t {
	public:
		inline host_t() throw() : string_t() { }

		inline void check(in_t::ptr_t &ptr) {
			for(in_t::ptr_t p = *this; p; ++p)
				if(*p == ':')
					config::error(ptr, "':' in hostname");
		}

		inline host_t(string_t const &string) throw() : string_t(string) { }

		inline ~host_t() throw() { }

		static inline cmp_t cmp(host_t const &h1, host_t const &h2) {
			return string_t::cmp<lower_t>(h1, h2);
		}
	};

	struct host_config_t {
		config::objptr_t<acl_t> acl;
		config::objptr_t<p3p_t> p3p;
		path_switch_t path;

		inline host_config_t() throw() : acl(), p3p(), path() { }
		inline void check(in_t::ptr_t const &) const { }
		inline ~host_config_t() throw() { }
	};

private:
	struct host_data_t {
		acl_t const *acl;
		p3p_t const *p3p;
		paths_t paths;

		path_data_t wrong;

		inline host_data_t(host_config_t const &config) :
			acl(config.acl), p3p(config.p3p), paths(config.path), wrong() { }

		inline ~host_data_t() throw() { }

		inline void init() {
			paths.init();
			wrong.init();
		}

		inline void stat_print() const {
			paths.stat_print();
			stat::ctx_t ctx(CSTR("*"));
			wrong.stat_print();
		}
	};

	struct hosts_t : darray2_t<host_t, host_data_t> {
		item_t *any;

		inline hosts_t(
			config::switch_t<host_t, config::struct_t<host_config_t>> const &host_switch,
			config::struct_t<host_config_t> const &any_host
		) throw() :
			darray2_t<host_t, host_data_t>(host_switch), any(NULL) {

			any = new item_t(host_t(STRING("*")), any_host);
		}

		inline ~hosts_t() throw() { delete any; }

		inline item_t *lookup(string_t const &str) const {
			size_t il = 0, ih = size;

			while(il < ih) {
				size_t i = (il + ih) / 2;
				cmp_t res = string_t::cmp<lower_t>(str, items[i]->key);

				if(res) return items[i];
				if(res.is_less()) ih = i;
				else il = i + 1;
			}

			return any;
		}

		inline void init() {
			for(size_t i = 0; i < size; ++i) {
				item_t *item = items[i];
				item->val.init();
			}

			any->val.init();
		}

		inline void stat_print() const {
			for(size_t i = 0; i < size; ++i) {
				item_t *item = items[i];

				stat::ctx_t ctx(item->key.str(), 2, item->val.paths.size + 1);
				item->val.stat_print();
			}

			stat::ctx_t ctx(any->key.str(), 2, any->val.paths.size + 1);
			any->val.stat_print();
		}

		friend class proto_http_t;
	};

	struct loggers_t : sarray1_t<logger_t *> {
		inline loggers_t(
			config::list_t<config::objptr_t<logger_t>> const &list
		) throw() :
			sarray1_t<logger_t *>(list) { }

		inline ~loggers_t() throw() { }

		inline void init(string_t const &name) {
			for(size_t i = 0; i < size; ++i)
				items[i]->init(name);
		}

		inline void stat_print(string_t const &name) const {
			stat::ctx_t ctx(CSTR("loggers"), 1);
			for(size_t i = 0; i < size; ++i)
				items[i]->stat_print(name);
		}

		inline void run(string_t const &name) const {
			for(size_t i = 0; i < size; ++i)
				items[i]->run(name);
		}

		inline void fini(string_t const &name) {
			for(size_t i = 0; i < size; ++i)
				items[i]->fini(name);
		}

		inline void commit(request_t const &request, reply_t const &reply) const {
			for(size_t i = 0; i < size; ++i)
				items[i]->commit(request, reply);
		}
	};

	acl_t const *acl;
	p3p_t const *p3p;
	http::limits_t const request_limits;
	string_t host_default;
	hosts_t hosts;
	loggers_t loggers;

	virtual void do_init() {
		hosts.init();
		loggers.init(name);
	}

	virtual void do_stat_print() const {
		{
			stat::ctx_t ctx(CSTR("hosts"), 1);
			hosts.stat_print();
		}
		loggers.stat_print(name);
	}

	virtual void do_run() const {
		//hosts.run();
		loggers.run(name);
	}

	virtual void do_fini() {
		//hosts.fini();
		loggers.fini(name);
	}

	virtual bool request_proc(
		in_t::ptr_t &ptr, out_t &out, netaddr_t const &, netaddr_t const &
	) const;

public:
	struct config_t {
		config_binding_type_ref(handler_t);
		config_binding_type_ref(p3p_t);
		config_binding_type_ref(logger_t);

		config::objptr_t<acl_t> acl;
		config::objptr_t<p3p_t> p3p;
		config::struct_t<http::limits_t::config_t> request_limits;
		string_t host_default;
		config::list_t<config::objptr_t<logger_t>> loggers;
		config::switch_t<host_t, config::struct_t<host_config_t>> host;
		config::struct_t<host_config_t> any_host;

		inline config_t() throw() :
			acl(), p3p(),
			request_limits(64 * sizeval::kilo, 128, 8 * sizeval::kilo, 16 * sizeval::mega),
			loggers() { }

		inline void check(in_t::ptr_t const &) const { }

		inline ~config_t() throw() { }
	};

	inline proto_http_t(string_t const &_name, config_t const &config) :
		proto_t(_name), acl(config.acl), p3p(config.p3p),
		request_limits(config.request_limits),
		host_default(config.host_default),
		hosts(config.host, config.any_host), loggers(config.loggers) { }

	inline ~proto_http_t() throw() { }
};

namespace proto_http {
namespace path {
config_binding_sname_sub(proto_http_t, path);
config_binding_value_sub(proto_http_t, path, acl);
config_binding_value_sub(proto_http_t, path, p3p);
config_binding_value_sub(proto_http_t, path, handler);
}

namespace host {
config_binding_sname_sub(proto_http_t, host);
config_binding_value_sub(proto_http_t, host, acl);
config_binding_value_sub(proto_http_t, host, p3p);
config_binding_value_sub(proto_http_t, host, path);
}

config_binding_sname(proto_http_t);
config_binding_value(proto_http_t, request_limits);
config_binding_type(proto_http_t, handler_t);
config_binding_value(proto_http_t, acl);
config_binding_type(proto_http_t, p3p_t);
config_binding_value(proto_http_t, p3p);
config_binding_value(proto_http_t, host_default);
config_binding_type(proto_http_t, logger_t);
config_binding_value(proto_http_t, loggers);
config_binding_value(proto_http_t, host);
config_binding_value(proto_http_t, any_host);
config_binding_cast(proto_http_t, proto_t);
config_binding_ctor(proto_t, proto_http_t);

} // namespace proto_http

bool proto_http_t::request_proc(
	in_t::ptr_t &ptr, out_t &out,
	netaddr_t const &local_addr, netaddr_t const &remote_addr
) const {
	if(!ptr) return false;

// RFC 2616, 4.1:
// "... servers SHOULD ignore any empty
// line(s) received where a Request-Line is expected."

	while(*ptr == '\r' || *ptr == '\n')
		if(!++ptr) return false;

	p3p_t const *p3p_current = p3p;

	request_t request(local_addr, remote_addr);
	reply_t reply(request);

	host_data_t *host_data = NULL;
	path_data_t *path_data = NULL;
	bq_thr_t *bq_thr_orig = bq_thr_get();

	try {
		request.parse(ptr, request_limits);

		request.settime();

		request.prepare(host_default);

		if(acl && !acl->check(remote_addr))
			throw http::exception_t(http::code_403, "Access denied (common)");

		hosts_t::item_t *host_item = hosts.lookup(request.host);
		if(!host_item)
			throw http::exception_t(http::code_403, "No vhost found");

		host_data = &host_item->val;

		log::handler_t handler_host(host_item->key);

		if(host_item->val.p3p)
			p3p_current = host_item->val.p3p;

		if(host_item->val.acl && !host_item->val.acl->check(remote_addr))
			throw http::exception_t(http::code_403, "Access denied (vhost)");

		paths_t const &paths = host_item->val.paths;

		paths_t::item_t *path_item = paths.lookup(request.full_path);
		if(!path_item)
			throw http::exception_t(http::code_404, "No handler found");

		path_data = &path_item->val;

		log::handler_t handler_path(path_item->key ?: STRING("/"));

		if(path_item->val.p3p)
			p3p_current = path_item->val.p3p;

		if(path_item->val.acl && !path_item->val.acl->check(remote_addr))
			throw http::exception_t(http::code_403, "Access denied (handler)");

		request.path = request.full_path.substring_tail(path_item->key.size());

		if(!request.path)
			request.path = STRING("/");

		handler_t const &handler = *(path_item->val.handler);

		handler.proc(request, reply);
	}
	catch(http::exception_t const &ex) {
		str_t msg = ex.msg();
		log_warning("%u %.*s", ex.code(), (int)msg.size(), msg.ptr());
		reply.set(new reply_t::error_content_t(ex.code()));
	}
	catch(exception_t const &ex) {
		reply.set(new reply_t::error_content_t(http::code_500));
	}
	catch(...) {
		log_error("unknown exception");
		reply.set(new reply_t::error_content_t(http::code_500));
	}

	if(!request.time.is_real())
		request.settime();

	if(path_data)
		++path_data->stat.count();
	else if(host_data)
		++host_data->wrong.stat.count();
	else
		++hosts.any->val.wrong.stat.count();

	reply.reply_time = (timeval::current() - request.time);

	server_t server(p3p_current);

	out.ctl(1);

	reply.print(out, server); // out.flush_all() inside

	out.ctl(0);

	reply.postout();

	loggers.commit(request, reply);

	if(bq_thr_orig != bq_thr_get()) {
		interval_t prio = interval::zero;
		bq_thr_orig->switch_to(prio);
	}

	if(request.keepalive) {
		while(ptr.pending() && (*ptr == '\r' || *ptr == '\n')) ++ptr;
	}

	return request.keepalive;
}

}} // namespace phantom::io_stream

namespace pd { namespace config {

typedef phantom::io_stream::proto_http_t::host_t host_t;

template<>
void helper_t<host_t>::parse(in_t::ptr_t &ptr, host_t &host) {
	helper_t<string_t>::parse(ptr, host);
	host.check(ptr);
}

template<>
void helper_t<host_t>::print(out_t &out, int off, host_t const &host) {
	helper_t<string_t>::print(out, off, host);
}

template<>
void helper_t<host_t>::syntax(out_t &out) {
	helper_t<string_t>::syntax(out);
}

}} // namespace pd::config
