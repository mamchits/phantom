// This file is part of the pd::http library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "server.H"

#include <pd/base/exception.H>

namespace pd { namespace http {

static inline bool isspace(char c) {
	switch(c) {
		case ' ': case '\t': case '\r': case '\n': return true;
	}
	return false;
}

static inline bool hexdigit(char c, unsigned int &res) {
	switch(c) {
		case '0'...'9': res = c - '0'; return true;
		case 'a'...'f': res = 10 + (c - 'a'); return true;
		case 'A'...'F': res = 10 + (c - 'A'); return true;
	}
	return false;
}

// RFC 2396, A:
//
// alphanum      = alpha | digit
// alpha         = lowalpha | upalpha
//
// lowalpha = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" |
//            "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" |
//            "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
// upalpha  = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
//            "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
//            "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
// digit    = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" |
//            "8" | "9"

static inline bool uri_alphanum_char(char c) {
	switch(c) {
		case 'a'...'z': case 'A'...'Z': case '0'...'9':
			return true;
	}
	return false;
}

// RFC 3986, 2.2
//
// reserved    = gen-delims / sub-delims
//
// gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
//
// sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
//             / "*" / "+" / "," / ";" / "="

static inline bool uri_gen_delim_char(char c) {
	switch(c) {
		case ':': case '/': case '?': case '#': case '[': case ']': case '@':
			return true;
	}
	return false;
}

static inline bool uri_sub_delim_char(char c) {
	switch(c) {
		case '!': case '$': case '&': case '\'': case '(': case ')':
		case '*': case '+': case ',': case ';': case '=':
			return true;
	}
	return false;
}

static inline bool uri_reserved_char(char c) {
	return uri_gen_delim_char(c) || uri_sub_delim_char(c);
}

// RFC 3986, 2.3
//
// unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"

static inline bool uri_unreserved_char(char c) {
	if(uri_alphanum_char(c)) return true;
	switch(c) {
		case '-': case '.': case '_': case '~':
			return true;
	}
	return false;
}

// RFC 3986, 3.2.2
//
// host        = IP-literal / IPv4address / reg-name
// IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
// IPvFuture  = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
// reg-name    = *( unreserved / pct-encoded / sub-delims )
//
// - We don't support pct-encoded hostnames.
// - We don't support '[address]' form hostnames.
//                                            - Eugene

static inline bool uri_host_char(char c) {
	return uri_unreserved_char(c) || uri_sub_delim_char(c);
}

// RFC 3986, 3.3
//
// path-absolute = "/" [ segment-nz *( "/" segment ) ]
// segment       = *pchar
// segment-nz    = 1*pchar
// pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"

static inline bool uri_path_char(char c) {
	if(uri_unreserved_char(c) || uri_sub_delim_char(c)) return true;
	switch(c) {
		case '/': case ':': case '@': case '%':
			return true;
	}
	return false;
}

// RFC 3986, 3.4
//
// query       = *( pchar / "/" / "?" )

static inline bool uri_query_char(char c) {
	if(uri_unreserved_char(c) || uri_sub_delim_char(c)) return true;
	switch(c) {
		case '/': case '?': case '%':
			return true;
	}
	return false;
}

void remote_request_t::parse(in_t::ptr_t &ptr, limits_t const &limits) {
	in_t::ptr_t ptr0 = ptr;

	try {
		switch(*ptr) {
			case 'H': case 'h':
				if(ptr.match<lower_t>(CSTR("HEAD "))) method = method_head;
				break;
			case 'G': case 'g':
				if(ptr.match<lower_t>(CSTR("GET "))) method = method_get;
				break;
			case 'P': case 'p':
				if(ptr.match<lower_t>(CSTR("POST "))) method = method_post;
				break;
		}

		if(method == method_undefined) {
			size_t limit = 256;
			if(!ptr.scan(" \r\n", 3, limit) || *(ptr++) != ' ')
				throw exception_t(code_400, "illegal request line");
		}

		{
			size_t limit = limits.line;

			if(*ptr == 'h') {
				if(
					!ptr.match<ident_t>(CSTR("http://")) &&
					!ptr.match<ident_t>(CSTR("https://"))
				)
					throw exception_t(code_400, "illegal request URI");

				in_t::ptr_t ptr1 = ptr;

				if(!ptr.scan("/? \r\n", 5, limit))
					throw exception_t(code_414, "request URI too large");

				if(ptr == ptr1)
					throw exception_t(code_400, "empty host name");

				host = string_t(ptr1, ptr - ptr1);
			}

			if(*ptr == '/') {
				in_t::ptr_t ptr1 = ptr;

				if(!ptr.scan("? \r\n", 4, limit))
					throw exception_t(code_414, "request URI too large");

				uri_path = in_segment_t(ptr1, ptr - ptr1);
			}
			else
				throw exception_t(code_400, "Not a path");

			if(*ptr == '?') {
				in_t::ptr_t ptr1 = ++ptr;

				if(!ptr.scan(" \r\n", 3, limit))
					throw exception_t(code_414, "request URI too large");

				uri_args = in_segment_t(ptr1, ptr - ptr1);
			}
		}

		eol_t eol;

		switch(*ptr) {
			case '\r': case '\n':
				version = version_0_9;
				if(method == method_get) {
					line = in_segment_t(ptr0, ptr - ptr0);
					all = in_segment_t(ptr0, ptr - ptr0);
					return;
				}
				throw exception_t(code_400, "unknown method for HTTP/0.9");
			case ' ': {
				++ptr;

				version = parse_version(ptr);
				if(version == version_undefined || !(*ptr == '\r' || *ptr == '\n'))
					throw exception_t(code_400, "illegal HTTP version");

				line = in_segment_t(ptr0, ptr - ptr0);

				if(!eol.set(ptr))
					throw exception_t(code_400, "junk at end of request line");

				if(method == method_undefined)
					throw exception_t(code_501, "unknown method");
			}
			break;
			default:
				fatal("internal error");
		}

		header.parse(ptr, eol, limits);

		full_path = path_decode(uri_path);

		entity.parse(ptr, eol, header, limits, false);

		all = in_segment_t(ptr0, ptr - ptr0);
	}
	catch(http::exception_t const &) {
		keepalive = false;
		ptr.seek_end();

		all = in_segment_t(ptr0, ptr - ptr0);

		throw;
	}
}

void remote_request_t::prepare(string_t const &host_default) {
	if(!host) {
		in_segment_t const *val = header.lookup(CSTR("host"));

		if(val) {
			in_t::ptr_t p0 = *val;
			while(p0 && isspace(*p0)) ++p0;
			in_t::ptr_t p1 = p0;
			while(p1 && !isspace(*p1) && (*p1 != ':')) ++p1;

			host = string_t(p0, p1 - p0);
		}
		else {
			if(version >= version_1_1)
				throw exception_t(code_400, "missing Host request-header with >= HTTP/1.1");

			host = host_default;
		}
	}

	{
		in_segment_t const *val = header.lookup(CSTR("connection"));

		if(val) {
			if(token_find(*val, CSTR("close")))
				keepalive = false;
			else if(token_find(*val, CSTR("keep-alive")))
				keepalive = true;
		}
		else
			keepalive = (version >= version_1_1);
	}
}

void local_reply_t::content_t::postout() const { }

string_t local_reply_t::content_t::ext_info(string_t const &) const {
	return string_t::empty;
}

local_reply_t::content_t::~content_t() throw() { }

code_t local_reply_t::error_content_t::code() const throw() {
	return error_code;
}

void local_reply_t::error_content_t::print_header(
	out_t &out, server_t const &
) const {
	out(CSTR("Content-Type: text/plain; charset=UTF-8")).crlf();
}

ssize_t local_reply_t::error_content_t::size() const throw() {
	return descr.size() + 1;
}

bool local_reply_t::error_content_t::print(out_t &out) const {
	out(descr).lf(); return true;
}

local_reply_t::error_content_t::~error_content_t() throw() { }

class out_chunked_t : public out_t {
	out_t &backend;

	virtual void flush();
	virtual out_t &ctl(int i);
	virtual out_t &sendfile(int fd, off_t &offset, size_t &size);

	void the_end() {
		flush_all();
		backend('0').crlf().crlf();
	}

public:
	inline out_chunked_t(char *_data, size_t _size, out_t &_backend) throw() :
		out_t(_data, _size), backend(_backend) { }

	inline ~out_chunked_t() throw() { safe_run(*this, &out_chunked_t::the_end); }
};

void out_chunked_t::flush() {
	backend.print(used(), "x").crlf();

	if(rpos < size) {
		char const *_data= data + rpos;
		size_t _size = ((wpos > rpos) ? wpos : size) - rpos;
		backend(str_t(_data, _size));
	}

	if(wpos > 0 && wpos <= rpos) {
		backend(str_t(data, wpos));
	}

	backend.crlf();

	wpos = 0;
	rpos = size;
}

out_t &out_chunked_t::ctl(int i) {
	backend.ctl(i);
	return *this;
}

out_t &out_chunked_t::sendfile(int from_fd, off_t &_offset, size_t &_len) {
	return out_t::sendfile(from_fd, _offset, _len);
}

void local_reply_t::print(out_t &out, server_t const &server) {
	if(!content) {
		log_error("no reply generated");
		content = new error_content_t(code_500);
	}

	ssize_t size = content->size();
	code_t code = content->code();

	if(size < 0 && request.version < version_1_1)
		request.keepalive = false;

	if(request.version >= version_1_0) {
		out
			(CSTR("HTTP/1.1 ")).print((uint16_t)code)(' ')
			(code_descr(code)).crlf()
		;

		request.print_header(out);
		server.print_header(out);
		content->print_header(out, server);

		// FIXME (don't send C-L with some reply codes)
		if(size >= 0)
			out(CSTR("Content-Length: ")).print(size).crlf();
		else if(request.version > version_1_0)
			out(CSTR("Transfer-Encoding: chunked")).crlf();

		if(request.keepalive) {
			if(request.version < version_1_1)
				out(CSTR("Connection: keep-alive")).crlf();
		}
		else
			out(CSTR("Connection: close")).crlf();

		out.crlf();
	}

	if(size < 0 && request.version > version_1_0) {
		char buf[64U * 1024U]; // FIXME: configure it
		out_chunked_t __out(buf, sizeof(buf), out);

		if(!content->print(__out))
			request.keepalive = false;
	}
	else {
		if(!content->print(out))
			request.keepalive = false;
	}

	out.flush_all();
}

string_t path_decode(in_segment_t const &uri_path) {
	in_t::ptr_t ptr0 = uri_path;
	in_t::ptr_t ptr = ptr0;
	enum { none = 0, slash, dot, dotdot } state = none;

	while(ptr) {
		char c = *ptr;

		if(!c)
			throw exception_t(code_404, "Zero in path");

		if(c == '%')
			goto long_path;

		switch(state) {
			case none:
				if(c == '/') state = slash;

				break;
			case slash:
				if(c == '/') goto long_path;
				if(c == '.') state = dot;
				else state = none;

				break;
			case dot:
				if(c == '/') goto long_path;
				if(c == '.') state = dotdot;
				else state = none;

				break;
			case dotdot:
				if(c == '/') goto long_path;
				state = none;

				break;
		}
		++ptr;
	}

	if(state == none || state == slash)
		return string_t(uri_path);

long_path:
	in_t::ptr_t bound = ptr0 + uri_path.size();

	string_t::ctor_t ctor(uri_path.size());

	ctor(ptr0, ptr - ptr0);

	while(ptr) {
		char c = *ptr;

		if(c == '%') {
			unsigned int c0, c1;
			if(bound - ptr < 3 || !hexdigit(*(++ptr), c1) || !hexdigit(*(++ptr), c0))
				throw exception_t(code_400, "Wrong symbol in path");

			c = c1 * 16 + c0;
		}

		if(!c)
			throw exception_t(code_404, "Zero in path");

		switch(state) {
			case none:
				if(c == '/') state = slash;

				break;
			case slash:
				if(c == '/') ctor.rollback('/');
				else if(c == '.') state = dot;
				else state = none;

				break;
			case dot:
				if(c == '/') {
					ctor.rollback('/');
					state = slash;
				}
				else if(c == '.') state = dotdot;
				else state = none;

				break;
			case dotdot:
				if(c == '/') {
					ctor.rollback('/');
					ctor.rollback('/');
					state = slash;
				}
				else state = none;

				break;
		}

		ctor(c);
		++ptr;
	}

	if(state == dot) {
		ctor.rollback('/');
		ctor('/');
	}
	else if(state == dotdot) {
		ctor.rollback('/');
		ctor.rollback('/');
		ctor('/');
	}

	return ctor;
}

}} // namespace pd::http
