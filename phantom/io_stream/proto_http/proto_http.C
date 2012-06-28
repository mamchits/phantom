// This file is part of the phantom::io_stream::proto_http module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "handler.H"
#include "logger.H"
#include "path.H"

#include "../proto.H"
#include "../acl.H"
#include "../../module.H"

#include <pd/http/limits_config.H>

#include <pd/base/thr.H>
#include <pd/base/exception.H>
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

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

namespace p3p {
config_binding_sname(p3p_t);
config_binding_value(p3p_t, compact_value);
config_binding_value(p3p_t, reference);
config_binding_ctor_(p3p_t);
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

	typedef config::switch_t<path_t, config::struct_t<path_config_t> > path_switch_t;

	class host_t : public string_t {
	public:
		inline host_t() throw() : string_t() { }

		inline void check(in_t::ptr_t &ptr) {
			for(in_t::ptr_t p = *this; p; ++p)
				if(*p == ':')
					config::error(ptr, "':' in hostname");
		}

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

	struct server_t;

	class path_data_t;

private:
	acl_t const *acl;
	p3p_t const *p3p;
	http::limits_t const request_limits;
	string_t host_default;

	virtual bool request_proc(
		in_t::ptr_t &ptr, out_t &out, netaddr_t const &, netaddr_t const &
	);

	virtual void stat(out_t &out, bool clear);

	struct paths_t;

	struct hosts_t {
		struct item_t;

		size_t size;
		item_t *items;

		inline hosts_t() throw() : size(0), items(NULL) { }
		~hosts_t() throw();

		item_t const *lookup(string_t const &str) const;
		void stat(out_t &out, bool clear);

		friend class proto_http_t;
	} hosts;

	hosts_t::item_t *any_host;

	struct loggers_t {
		size_t size;
		logger_t **items;
		inline loggers_t() throw() : size(0), items(NULL) { }
		inline ~loggers_t() throw() { if(items) delete [] items; }

		inline void commit(request_t const &request, reply_t const &reply) const {
			for(size_t i = 0; i < size; ++i)
				items[i]->commit(request, reply);
		}
	} loggers;

public:
	struct config_t {
		config_binding_type_ref(handler_t);
		config_binding_type_ref(p3p_t);
		config_binding_type_ref(logger_t);

		config::objptr_t<acl_t> acl;
		config::objptr_t<p3p_t> p3p;

		config::struct_t<http::limits_t::config_t> request_limits;

		string_t host_default;

		config::list_t<config::objptr_t<logger_t> > loggers;

		config::switch_t<host_t, config::struct_t<host_config_t> > host;
		config::struct_t<host_config_t> any_host;

		inline config_t() throw() :
			acl(), p3p(),
			request_limits(64 * sizeval_kilo, 128, 8 * sizeval_kilo, 16 * sizeval_mega),
			loggers() { }

		inline void check(in_t::ptr_t const &) const { }

		inline ~config_t() throw() { }
	};

	proto_http_t(string_t const &, config_t const &);
	~proto_http_t() throw();
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
config_binding_ctor(proto_t, proto_http_t);

} // namespace proto_http

class proto_http_t::path_data_t {
	thr::spinlock_t spinlock;
	uint64_t count;

public:
	acl_t const *acl;
	p3p_t const *p3p;
	handler_t const *handler;

	inline path_data_t() throw() :
		spinlock(), count(0),
		acl(NULL), p3p(NULL), handler(NULL) { }

	inline void stat(out_t &out, bool clear) {
		uint64_t _count = ({
			thr::spinlock_guard_t guard(spinlock);
			uint64_t __count = count;
			if(clear) count = 0;
			__count;
		});

		out('\t').print(_count);
	}

	inline void update_stat() {
		thr::spinlock_guard_t guard(spinlock);
		++count;
	}

	inline path_data_t &operator=(path_config_t const &path_config) {
		acl = path_config.acl;
		p3p = path_config.p3p;
		handler = path_config.handler;

		return *this;
	}
};

struct proto_http_t::paths_t : proto_http::paths_t<path_data_t> {
	inline paths_t() throw() : proto_http::paths_t<path_data_t>() { }
	inline ~paths_t() throw() { }

	inline void stat(out_t &out, bool clear) {
		for(size_t i = 0; i < size; ++i) {
			item_t &item = items[i];
			out('\t')(item.path)('/')('\t');
			item.data.stat(out, clear);
			out.lf();
		}
	}
};

// ------------------------

struct proto_http_t::hosts_t::item_t {
	host_t host;
	acl_t const *acl;
	p3p_t const *p3p;

	paths_t paths;

	inline item_t() throw() :
		host(), acl(NULL), p3p(NULL), paths() { }

	inline void setup(host_config_t const &host_config) {
		acl = host_config.acl;
		p3p = host_config.p3p;
		paths.setup(host_config.path);
	}

	inline void stat(out_t &out, bool clear) {
		out(host).lf();
		paths.stat(out, clear);
	}

	inline ~item_t() throw() { }
};

proto_http_t::hosts_t::~hosts_t() throw() { if(items) delete [] items; }

proto_http_t::hosts_t::item_t const *proto_http_t::hosts_t::lookup(
	string_t const &str
) const {
	size_t il = 0, ih = size;

	while(il < ih) {
		size_t i = (il + ih) / 2;
		cmp_t res = string_t::cmp<lower_t>(str, items[i].host);

		if(res) return &items[i];
		if(res.is_less()) ih = i;
		else il = i + 1;
	}

	return NULL;
}

void proto_http_t::hosts_t::stat(out_t &out, bool clear) {
	for(size_t i = 0; i < size; ++i)
		items[i].stat(out, clear);
}

proto_http_t::proto_http_t(string_t const &, config_t const &config) :
	acl(config.acl), p3p(config.p3p),
	request_limits(config.request_limits),
	host_default(config.host_default),
	hosts(), any_host(NULL), loggers() {

	for(typeof(config.host.ptr()) hptr = config.host; hptr; ++hptr)
		++hosts.size;

	hosts.items = new hosts_t::item_t[hosts.size];

	hosts_t::item_t *ph = hosts.items;

	for(typeof(config.host.ptr()) hptr = config.host; hptr; ++hptr) {
		ph->host = hptr.key();
		ph->setup(hptr.val());
		++ph;
	}

	any_host = new hosts_t::item_t;
	any_host->setup(config.any_host);

	for(typeof(config.loggers.ptr()) lptr = config.loggers; lptr; ++lptr)
		++loggers.size;

	loggers.items = new logger_t *[loggers.size];

	size_t i = 0;
	for(typeof(config.loggers.ptr()) lptr = config.loggers; lptr; ++lptr)
		loggers.items[i++] = lptr.val();
}

proto_http_t::~proto_http_t() throw() { delete any_host; }

struct proto_http_t::server_t : http::server_t {
	p3p_t const *p3p;

	virtual void print_header(out_t &out) const;
	virtual bool filter_header(in_segment_t const &string) const;

public:
	inline server_t(p3p_t const *_p3p) throw() : p3p(_p3p) { }
	inline ~server_t() throw() { }
};

void proto_http_t::server_t::print_header(out_t &out) const {
	out(CSTR("Server: Phantom/0.0.0")).crlf();
	if(p3p)
		p3p->print_header(out);
}

bool proto_http_t::server_t::filter_header(in_segment_t const &key) const {
	if(key.cmp_eq<lower_t>(CSTR("server")))
		return false;

	if(p3p && key.cmp_eq<lower_t>(CSTR("p3p")))
		return false;

	return true;
}

bool proto_http_t::request_proc(
	in_t::ptr_t &ptr, out_t &out,
	netaddr_t const &local_addr, netaddr_t const &remote_addr
) {
	if(!ptr) return false;

// RFC 2616, 4.1:
// "... servers SHOULD ignore any empty
// line(s) received where a Request-Line is expected."

	while(*ptr == '\r' || *ptr == '\n')
		if(!++ptr) return false;

	p3p_t const *p3p_current = p3p;

	request_t request(local_addr, remote_addr);
	reply_t reply(request);

	path_data_t *path_data = NULL;
	bq_thr_t *bq_thr_orig = bq_thr_get();

	try {
		request.parse(ptr, request_limits);

		request.settime();

		request.prepare(host_default);

		if(acl && !acl->check(remote_addr))
			throw http::exception_t(http::code_403, "Access denied (common)");

		hosts_t::item_t const *host_item = hosts.lookup(request.host) ?: any_host;
		if(!host_item)
			throw http::exception_t(http::code_403, "No vhost found");

		if(host_item->p3p)
			p3p_current = host_item->p3p;

		if(host_item->acl && !host_item->acl->check(remote_addr))
			throw http::exception_t(http::code_403, "Access denied (vhost)");

		paths_t const &paths = host_item->paths;

		paths_t::item_t *path_item = paths.lookup(request.full_path);
		if(!path_item)
			throw http::exception_t(http::code_404, "No handler found");

		if(path_item->data.p3p)
			p3p_current = path_item->data.p3p;

		if(path_item->data.acl && !path_item->data.acl->check(remote_addr))
			throw http::exception_t(http::code_403, "Access denied (handler)");

		request.path = request.full_path.substring_tail(path_item->path.size());

		if(!request.path)
			request.path = STRING("/");

		path_data = &path_item->data;

		handler_t const &handler = *(path_item->data.handler);

		handler.proc(request, reply);
	}
	catch(http::exception_t const &ex) {
		str_t msg = ex.msg();
		log_warning("%u %.*s", ex.code(), (int)msg.size(), msg.ptr());
		reply.set(new reply_t::error_content_t(ex.code()));
	}
	catch(exception_t const &ex) {
		ex.log();
		reply.set(new reply_t::error_content_t(http::code_500));
	}
	catch(...) {
		log_error("unknown exception");
		reply.set(new reply_t::error_content_t(http::code_500));
	}

	if(!request.time.is_real())
		request.settime();

	if(path_data)
		path_data->update_stat();

	reply.reply_time = (timeval_current() - request.time);

	server_t server(p3p_current);

	out.ctl(1);

	reply.print(out, server); // out.flush_all() inside

	out.ctl(0);

	reply.postout();

	loggers.commit(request, reply);

	if(bq_thr_orig != bq_thr_get()) {
		interval_t prio = interval_zero;
		bq_thr_orig->switch_to(prio);
	}

	if(request.keepalive) {
		while(ptr.pending() && (*ptr == '\r' || *ptr == '\n')) ++ptr;
	}

	return request.keepalive;
}

void proto_http_t::stat(out_t &out, bool clear) {
	hosts.stat(out, clear);
	out(CSTR("<any_host>")).lf();
	any_host->stat(out, clear);
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
