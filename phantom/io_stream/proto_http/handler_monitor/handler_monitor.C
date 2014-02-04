// This file is part of the phantom::io_stream::proto_http::handler_monitor module.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../handler.H"

#include "../../../module.H"
#include "../../../obj.H"
#include "../../../stat.H"

#include <pd/base/size.H>
#include <pd/base/stat_ctx.H>

namespace phantom { namespace io_stream { namespace proto_http {

MODULE(io_stream_proto_http_handler_monitor);

class handler_monitor_t : public handler_t {
	size_t res_no;

	virtual void do_proc(request_t const &request, reply_t &reply) const;

	class content_t : public reply_t::content_t {
		size_t res_no;
		in_segment_t path;
		in_segment_t args;

		virtual http::code_t code() const throw() { return http::code_200; }
		virtual void print_header(out_t &out, http::server_t const &) const {
			out(CSTR("Content-Type: text/html; charset=utfi-8")).crlf();
		}
		virtual ssize_t size() const throw() { return -1; }
		virtual bool print(out_t &) const;
		virtual ~content_t() throw() { }
	public:
		inline content_t(
			size_t _res_no, in_segment_t const &_path, in_segment_t const &_args
		) :
			res_no(_res_no), path(_path), args(_args) { }
	};

public:
	struct config_t : handler_t::config_t {
		stat_id_t stat_id;

		inline config_t() throw() : handler_t::config_t(), stat_id() { }
		inline void check(in_t::ptr_t const &ptr) const {
			handler_t::config_t::check(ptr);

			if(!stat_id)
				config::error(ptr, "stat_id is required");
		}
		inline ~config_t() throw() { }
	};

	inline handler_monitor_t(string_t const &name, config_t const &config) throw() :
		handler_t(name, config), res_no(config.stat_id.res_no) { }

	inline ~handler_monitor_t() throw() { }
};

namespace handler_monitor {
config_binding_sname(handler_monitor_t);
config_binding_value(handler_monitor_t, stat_id);
config_binding_parent(handler_monitor_t, handler_t);
config_binding_ctor(handler_t, handler_monitor_t);
} // namespace handler_monitor


#define __s1 "  "
#define __s2 "    "
#define __s3 "      "
#define __s4 "        "
#define __s5 "          "
#define __s6 "            "

static inline bool hexdigit(char c, unsigned int &res) {
	switch(c) {
		case '0'...'9': res = c - '0'; return true;
		case 'a'...'f': res = 10 + (c - 'a'); return true;
		case 'A'...'F': res = 10 + (c - 'A'); return true;
	}
	return false;
}

string_t arg_parse(in_segment_t const &segment) {
	in_t::ptr_t ptr = segment;
	size_t limit = segment.size();

	if(!ptr.scan("%+", 2, limit)) {
		return string_t(segment);
	}

	in_t::ptr_t ptr0 = segment;
	size_t size = segment.size();

	string_t::ctor_t ctor(size);
	ctor(ptr0, ptr - ptr0);

	ptr0 += size;

	while(ptr) {
		char c = *ptr;

		if(c == '%') {
			unsigned int c0, c1;
			if(ptr0 - ptr < 3 || !hexdigit(*(++ptr), c1) || !hexdigit(*(++ptr), c0))
				throw http::exception_t(http::code_400, "Wrong symbol in arg");

			c = c1 * 16 + c0;
		}
		else if(c == '+')
			c = ' ';

		ctor(c);
		++ptr;
	}

	return ctor;
}

bool handler_monitor_t::content_t::print(out_t &out) const {
	unsigned int period = 0, savedperiod = 0;
	bool clear = false;
	bool json = false;
	string_t css;

	size_t count = objs_all(NULL);

	obj_t const *objs[count];

	objs_all(objs);
	bool checked[count];

	for(size_t i = 0; i < count; ++i)
		checked[i] = false;

	{
		in_t::ptr_t ptr = args;

		while(ptr) {
			if(*ptr == '%') {
				if(ptr.match<ident_t>(CSTR("%24clear=on"))) {
					clear = true;
				}
				else if(ptr.match<ident_t>(CSTR("%24json=on"))) {
					json = true;
				}
				else if(ptr.match<ident_t>(CSTR("%24all=on"))) {
					for(size_t i = 0; i < count; ++i)
						checked[i] = true;
				}
				else if(ptr.match<ident_t>(CSTR("%24period="))) {
					unsigned int _p;

					if(ptr.parse(_p))
						period = _p;
					else {
						log_error("illegal period value");
						break;
					}
				}
				else if(ptr.match<ident_t>(CSTR("%24css="))) {
					in_t::ptr_t ptr0 = ptr;
					size_t limit = sizeval::unlimited;

					if(!ptr.scan("&", 1, limit)) ptr.seek_end();
					css = arg_parse(in_segment_t(ptr0, ptr - ptr0));
				}
				else if(ptr.match<ident_t>(CSTR("%24savedperiod="))) {
					unsigned int _p;

					if(ptr.parse(_p))
						savedperiod = _p;
					else {
						log_error("illegal savedperiod value");
						break;
					}
				}
				else {
					log_error("unknown form parameter");
					break;
				}
			}
			else {
				size_t i;
				if(!(ptr.parse(i) && ptr.match<ident_t>(CSTR("=on")))) {
					log_error("unknown form parameter");
					break;
				}
				if(i >= count) {
					log_error("illegal object number");
					break;
				}

				checked[i] = true;
			}

			if(ptr) {
				if(*ptr == '&')
					++ptr;
				else {
					log_error("junk at the end of parameter");
					break;
				}
			}
		}
	}

	out(CSTR("<html>\n"));
	out(CSTR(__s1 "<head>\n"));
	out(CSTR(__s2 "<title>Phantom monitor</title>\n"));

	out(CSTR(__s2 "<style type=\"text/css\">\n"));
	out(CSTR(__s3 ".stat td { text-align: right; min-width: 3.5em }\n"));
	out(CSTR(__s3 ".stat th { text-align: center; min-width: 3.5em }\n"));
	out(CSTR(__s2 "</style>\n"));

	if(css)
		out(CSTR(__s2 "<link rel=stylesheet href=\""))(css)(CSTR("\" type=\"text/css\">\n"));

	out(CSTR(__s1 "</head>\n"));

	if(period) {
		out(CSTR(
			__s1 "<body onload=\"javascript:setTimeout('location.reload(false);',")).print(period * 1000)(CSTR(");\">\n"
		));
	}
	else {
		out(CSTR(__s1 "<body>\n"));
	}

	out
		(CSTR(__s2 "<div align=center>\n" __s3 "<form action=\""))
			(path)(CSTR("\" method=get name=\"Monitor params\">\n"))
	;

	if(period) {
		for(size_t i = 0; i < count; ++i) {
			if(checked[i])
				out(CSTR(__s4 "<input type=\"hidden\" name=\"")).print(i)(CSTR("\" value=\"on\">\n"));
		}

		if(clear)
			out(CSTR(__s4 "<input type=\"hidden\" name=\"$clear\" value=\"on\">\n"));

		if(json)
			out(CSTR(__s4 "<input type=\"hidden\" name=\"$json\" value=\"on\">\n"));

		if(css)
			out(CSTR(__s4 "<input type=\"hidden\" name=\"$css\" value=\""))(css)(CSTR("\">\n"));

		out(CSTR(__s4 "<input type=\"hidden\" name=\"$savedperiod\" value=\"")).print(period)(CSTR("\">\n"));

		out(CSTR(__s4 "<input type=\"submit\" value=\"Stop\">\n"));
	}
	else {
		out(CSTR(__s4 "<table class=\"ctl\">\n"));

		for(size_t i = 0; i < count; ++i) {
			out
				(CSTR(__s5 "<tr><td><input type=\"checkbox\" name=\""))
					.print(i)
					(checked[i] ? CSTR("\" checked>") : CSTR("\">"))
					(objs[i]->name)(CSTR("</td></tr>\n"))
			;
		}

		out
			(CSTR(__s5 "<tr><td><hr></td></tr>\n"))

			(CSTR(__s5 "<tr><td>\n"))

			(CSTR(__s6 "<input type=\"checkbox\" name=\"$all\""))
				(CSTR(">all &nbsp;\n"))
			(CSTR(__s6 "<input type=\"checkbox\" name=\"$clear\""))
				(clear ? CSTR(" checked") : CSTR(""))(CSTR(">clear &nbsp;\n"))
			(CSTR(__s6 "<input type=\"checkbox\" name=\"$json\""))
				(json ? CSTR(" checked") : CSTR(""))(CSTR(">json\n"))
			(CSTR(__s5 "</td></tr>\n"))

			(CSTR(__s5 "<tr><td>\n"))

			(CSTR(__s5 "<tr><td>CSS URL: <input type=\"text\" name=\"$css\" size=\"20\" value=\""))
				.print(css)(CSTR("\"></td></tr>\n"))

			(CSTR(__s5 "<tr><td>Period: <input type=\"text\" name=\"$period\" size=\"2\" value=\""))
				.print(savedperiod)(CSTR("\">sec <input type=\"submit\" value=\"Go!\"></td></tr>\n"))
			(CSTR(__s4 "</table>\n"))
		;
	}

	out(CSTR(__s3 "</form>\n" __s2 "</div>"));

	if(json) {
		out.lf()(CSTR(__s2 "<pre>\n"));

		{
			stat::ctx_t ctx(out, stat::ctx_t::json, res_no, clear);

			for(size_t i = 0; i < count; ++i) {
				if(checked[i])
					obj_stat_print(&objs[i], 1);
			}
		}

		out.lf()(CSTR(__s2 "</pre>"));
	}
	else {
		stat::ctx_t ctx(out, stat::ctx_t::html, res_no, clear, 2);

		for(size_t i = 0; i < count; ++i) {
			if(checked[i])
				obj_stat_print(&objs[i], 1);
		}
	}

	out.lf()(CSTR(__s1 "</body>\n</html>"));

	return true;
}

void handler_monitor_t::do_proc(request_t const &request, reply_t &reply) const {
	reply.set(new content_t(res_no, request.full_path, request.uri_args));
}

}}} // namespace phantom::io_stream::proto_http
