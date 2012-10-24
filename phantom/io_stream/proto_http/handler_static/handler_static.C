// This file is part of the phantom::io_stream::proto_http::handler_static module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "file_cache.I"
#include "file_types.H"

#include "../handler.H"
#include "../path.H"

#include "../../../module.H"

#include <pd/base/log.H>
#include <pd/base/size.H>
#include <pd/base/config_enum.H>

namespace phantom { namespace io_stream { namespace proto_http {

MODULE(io_stream_proto_http_handler_static);

namespace handler_static {

class path_translation_default_t : public path_translation_t {
	virtual string_t translate(
		string_t const &root, string_t const &path
	) const {
		return string_t::ctor_t(root.size() + path.size() + 1)(root)(path)('\0');
	}
public:
	inline path_translation_default_t() { }
	inline ~path_translation_default_t() throw() { }
};

static path_translation_default_t const default_path_translation;

} // namespace handler_static

class handler_static_t : public handler_t {
public:
	typedef handler_static::path_translation_t path_translation_t;
	typedef handler_static::file_type_t file_type_t;
	typedef handler_static::file_types_t file_types_t;
	typedef handler_static::file_t file_t;
	typedef handler_static::file_cache_t file_cache_t;

	struct opts_config_t {
		interval_t expires;
		config::enum_t<bool> allow_gzip;

		inline opts_config_t() throw() :
			expires(interval_inf), allow_gzip(false) { }

		inline void check(in_t::ptr_t const &) const { }
		inline ~opts_config_t() throw() { }
	};

	typedef paths_t<opts_config_t> opts_t;

private:
	virtual void do_proc(request_t const &request, reply_t &reply) const;

	class file_content_t;
	class method_not_allowed_content_t;

	file_types_t const &file_types;
	file_cache_t *cache;

	opts_t path_opts;
	opts_config_t default_opts;
	string_t charset;

public:
	struct config_t : handler_t::config_t {
		string_t root;
		config_binding_type_ref(path_translation_t);
		config::objptr_t<path_translation_t> path_translation;
		config_binding_type_ref(file_types_t);
		config::objptr_t<file_types_t> file_types;
		sizeval_t cache_size;
		interval_t cache_check_time;
		string_t charset;
		config::switch_t<path_t, config::struct_t<opts_config_t> > opts;
		config::struct_t<opts_config_t> default_opts;

		inline config_t() throw() :
			handler_t::config_t(),
			root(), path_translation(), file_types(), cache_size(8 * sizeval_kilo),
			cache_check_time(interval_second), charset(STRING("UTF-8")),
			opts(), default_opts() { }

		inline void check(in_t::ptr_t const &ptr) const {
			handler_t::config_t::check(ptr);

			if(!root)
				config::error(ptr, "root is required");

			if(!file_types) {
				config::error(ptr, "file_types is required");
			}
		}

		inline ~config_t() throw() { }
	};

	inline handler_static_t(string_t const &name, config_t const &config) :
		handler_t(name, config), file_types(*config.file_types),
		path_opts(), default_opts(config.default_opts),
		charset(config.charset) {

		path_translation_t const *translation = config.path_translation;
		if(!translation)
			translation = &handler_static::default_path_translation;

		cache = new file_cache_t(
			config.cache_size, config.root, *translation, config.cache_check_time
		);

		path_opts.setup(config.opts);
	}

	inline ~handler_static_t() throw() {
		if(cache)
			delete cache;
	}
};

namespace handler_static {

namespace opts {
config_binding_sname_sub(handler_static_t, opts);
config_binding_value_sub(handler_static_t, opts, expires);
config_binding_value_sub(handler_static_t, opts, allow_gzip);
}

config_binding_sname(handler_static_t);
config_binding_value(handler_static_t, root);
config_binding_type(handler_static_t, path_translation_t);
config_binding_value(handler_static_t, path_translation);
config_binding_value(handler_static_t, cache_size);
config_binding_value(handler_static_t, cache_check_time);
config_binding_value(handler_static_t, charset);
config_binding_type(handler_static_t, file_types_t);
config_binding_value(handler_static_t, file_types);
config_binding_value(handler_static_t, opts);
config_binding_value(handler_static_t, default_opts);
config_binding_parent(handler_static_t, handler_t, 1);
config_binding_ctor(handler_t, handler_static_t);

} // namespace handler_static

class handler_static_t::file_content_t :
	public reply_t::content_t {

	ref_t<file_t> file;
	request_t const &request;
	bool get;
	bool modified;
	bool gz;
	interval_t expires;
	file_type_t const &ft;
	string_t charset;

	virtual ~file_content_t() throw() { }

	virtual http::code_t code() const throw() {
		return modified ? http::code_200 : http::code_304;
	}

	virtual void print_header(out_t &out, http::server_t const &) const {
		if(modified) {
			out(CSTR("Last-Modified: "))(file->mtime_string).crlf();

			if(ft.mime_type) {
				out(CSTR("Content-Type: "))(ft.mime_type);
				if(ft.need_charset)
					out(CSTR("; charset="))(charset);
				out.crlf();
			}

			if(gz) {
				out(CSTR("Content-Encoding: gzip")).crlf();
			}
		}

		if(expires.is_real())
			out(CSTR("Expires: "))(http::time_string(request.time + expires)).crlf();
	}

	virtual ssize_t size() const throw() { return modified ? file->size : 0; }

	virtual bool print(out_t &out) const {
		if(get && modified) {
			size_t size = file->size;
			off_t offset = 0;
			out.sendfile(file->fd, offset, size);
			if(size) {
				log_error("File \"%s\" truncated", file->sys_name_z.ptr());
				return false;
			}
		}
		return true;
	}

public:
	inline file_content_t(
		ref_t<file_t> _file, bool _get, bool _gz,
		request_t const &_request, interval_t _expires,
		file_type_t const &_ft, string_t const &_charset
	) :
		file(_file), request(_request), get(_get),
		modified(true), gz(_gz), expires(_expires),
		ft(_ft), charset(_charset) {

		in_segment_t const *val = request.header.lookup(CSTR("if-modified-since"));

		if(val) {
			timeval_t time;
			if(http::time_parse(*val, time))
				modified = (file->mtime > time);
			else {
				string_t str(*val);

				size_t len = str.size();
				char const *ptr = str.ptr();

				while(len && ptr[len - 1] < ' ') // '\r' or '\n'
					--len;

				log_warning("Cannot parse time '%.*s'", (int)len, ptr);
			}
		}
	}
};

class handler_static_t::method_not_allowed_content_t :
	public reply_t::content_t {

	virtual ~method_not_allowed_content_t() throw() { }

	virtual http::code_t code() const throw() { return http::code_405; }

	virtual void print_header(out_t &out, http::server_t const &) const {
		out(CSTR("Allow: GET, HEAD")).crlf();
	}

	virtual ssize_t size() const throw() { return 0; }
	virtual bool print(out_t &) const { return true; }

public:
	inline method_not_allowed_content_t() throw() { }
};

static inline bool is_space(char c) {
	switch(c) {
		case ' ': case '\t': case '\r': case '\n': return true;
	}
	return false;
}

static str_t const decayed_agent_prefix = CSTR("Mozilla/4.0 (compatible; MSIE ");

static bool agent_gzip_capable(handler_t::request_t const &request) {
	in_segment_t const *val;

	val = request.header.lookup(CSTR("user-agent"));
	if(val) {
		in_t::ptr_t p = *val;
		while(p && is_space(*p)) ++p;

		if(
			p.match<ident_t>(decayed_agent_prefix) &&
			(!p || *(p++) < '7') && (!p || *(p++) == '.')
		)
			return false;
	}

	val = request.header.lookup(CSTR("accept-encoding"));

	if(val && http::token_find(*val, CSTR("gzip")))
		return true;

	return false;
}

void handler_static_t::do_proc(request_t const &request, reply_t &reply) const {
	opts_config_t const *opts = &default_opts;

	opts_t::item_t const *item = path_opts.lookup(request.path);
	if(item)
		opts = &item->data;

	bool get = true;
	switch(request.method) {
		case http::method_head: get = false;
		case http::method_get:
		{
			file_type_t const *ft = NULL;
			{
				char const *ptr0 = request.path.ptr();
				size_t len = request.path.size();
				for(char const *ptr = ptr0 + len; ptr > ptr0; --ptr) {
					if(ptr[-1] == '/')
						break;
					if(ptr[-1] == '.') {
						ft = file_types.lookup(request.path.substring_tail(ptr - ptr0));
						break;
					}
				}
			}

			if(!ft)
				throw http::exception_t(http::code_404, "File type not found");

			if(opts->allow_gzip && (ft->allow_gzip) && agent_gzip_capable(request)) {
				string_t path =
					string_t::ctor_t(request.path.size() + 3)(request.path)('.')('g')('z');

				ref_t<file_t> file = cache->find(path);
				if(*file) {
					reply.set(
						new file_content_t(
							file, get, true, request, opts->expires, *ft, charset
						)
					);
					return;
				}
			}

			ref_t<file_t> file = cache->find(request.path);
			if(!*file)
				throw http::exception_t(http::code_404, "File not found");

			reply.set(
				new file_content_t(file, get, false, request, opts->expires, *ft, charset)
			);
		}
		break;
		default:
			reply.set(new method_not_allowed_content_t());
	}
}

}}} // namespace phantom::io_stream::proto_http
