// This file is part of the pd::http library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/string.H>
#include <pd/base/time.H>
#include <pd/base/list.H>
#include <pd/base/exception.H>

#pragma GCC visibility push(default)

namespace pd { namespace http {

enum method_t {
	method_undefined = 0,
	method_head,
	method_get,
	method_post
};

enum version_t {
	version_undefined = 0,
	version_0_9,
	version_1_0,
	version_1_1
};

#define HTTP_CODE_LIST \
	HTTP_CODE(100, "Continue") \
	HTTP_CODE(101, "Switching Protocols") \
	HTTP_CODE(200, "OK") \
	HTTP_CODE(201, "Created") \
	HTTP_CODE(202, "Accepted") \
	HTTP_CODE(203, "Non-Authoritative Information") \
	HTTP_CODE(204, "No Content") \
	HTTP_CODE(205, "Reset Content") \
	HTTP_CODE(206, "Partial Content") \
	HTTP_CODE(300, "Multiple Choices") \
	HTTP_CODE(301, "Moved Permanently") \
	HTTP_CODE(302, "Found") \
	HTTP_CODE(303, "See Other") \
	HTTP_CODE(304, "Not Modified") \
	HTTP_CODE(305, "Use Proxy") \
	HTTP_CODE(307, "Temporary Redirect") \
	HTTP_CODE(400, "Bad Request") \
	HTTP_CODE(401, "Unauthorized") \
	HTTP_CODE(402, "Payment Required") \
	HTTP_CODE(403, "Forbidden") \
	HTTP_CODE(404, "Not Found") \
	HTTP_CODE(405, "Method Not Allowed") \
	HTTP_CODE(406, "Not Acceptable") \
	HTTP_CODE(407, "Proxy Authentication Required") \
	HTTP_CODE(408, "Request Time-out") \
	HTTP_CODE(409, "Conflict") \
	HTTP_CODE(410, "Gone") \
	HTTP_CODE(411, "Length Required") \
	HTTP_CODE(412, "Precondition Failed") \
	HTTP_CODE(413, "Request Entity Too Large") \
	HTTP_CODE(414, "Request-URI Too Large") \
	HTTP_CODE(415, "Unsupported Media Type") \
	HTTP_CODE(416, "Requested range not satisfiable") \
	HTTP_CODE(417, "Expectation Failed") \
	HTTP_CODE(500, "Internal Server Error") \
	HTTP_CODE(501, "Not Implemented") \
	HTTP_CODE(502, "Bad Gateway") \
	HTTP_CODE(503, "Service Unavailable") \
	HTTP_CODE(504, "Gateway Time-out") \
	HTTP_CODE(505, "HTTP Version not supported")

enum code_t {
	code_undefined = 0,
#define HTTP_CODE(x,y) \
	code_##x = x,

	HTTP_CODE_LIST

#undef HTTP_CODE
};

string_t code_descr(code_t code);

class exception_t : public pd::exception_t {
	code_t _code;
	char const *_msg;

public:
	inline exception_t(code_t __code, char const *__msg) throw() :
		pd::exception_t(), _code(__code), _msg(__msg) { }

	inline code_t code() const throw() { return _code; }

	virtual void log() const;
	virtual str_t msg() const;
	virtual ~exception_t() throw();
};

class eol_t {
	char c1, c2;

public:
	inline eol_t() : c1('\0'), c2('\0') { }
	inline eol_t(char _c1, char _c2) : c1(_c1), c2(_c2) { }
	inline ~eol_t() { }

	inline bool set(in_t::ptr_t &ptr) {
		switch(*(ptr++)) {
			case '\r':
				if(*ptr == '\n') { ++ptr; c1 = '\r'; c2 = '\n'; return true; }
				break;
			case '\n':
				if(*ptr == '\r') { ++ptr; c1 = '\n'; c2 = '\r'; return true; }
				else { c1 = '\n'; return true; }
				break;
		}
		return false;
	}

	inline bool check(in_t::ptr_t &ptr) const {
		in_t::ptr_t ptr0 = ptr;

		if(*(ptr0++) != c1)
			return false;

		if(c2 && *(ptr0++) != c2)
			return false;

		ptr = ptr0;
		return true;
	}
};

struct limits_t {
	size_t const line, field_num, field_size, entity_size;

	inline limits_t(
		size_t _line, size_t _field_num, size_t _field_size, size_t _entity_size
	) throw() :
		line(_line), field_num(_field_num),
		field_size(_field_size), entity_size(_entity_size) { }

	struct config_t;

	limits_t(config_t const &);
};

class mime_header_t {
	struct item_t {
		item_t *first;
		item_t *next;
		in_segment_t key, val;

		inline item_t() throw() : first(NULL), next(NULL), key(), val() { }
		inline ~item_t() throw() { }
	};

	item_t *items;
	size_t count;

	inline size_t index(str_t const &key) const {
		assert(count > 0);
		return key.fnv<lower_t>() % count;
	}

	inline size_t index(in_segment_t const &key) {
		assert(count > 0);
		return key.fnv<lower_t>() % count;
	}

	item_t const *__lookup(str_t key) const;

	void parse_item(
		in_t::ptr_t &ptr, eol_t const &eol, limits_t const &limits, size_t depth
	);

public:
	inline void clear() throw() {
		if(items) {
			delete[] items;
			items = NULL;
		}
		count = 0;
	}

	inline mime_header_t() throw() : items(NULL), count(0) { }

	inline ~mime_header_t() throw() { clear(); }

	inline void parse(
		in_t::ptr_t &ptr, eol_t const &eol, limits_t const &limits
	) {
		clear();
		parse_item(ptr, eol, limits, 0);
	}

	inline size_t size() const { return count; }
	inline in_segment_t const &key(size_t i) const { return items[i].key; }
	inline in_segment_t const &val(size_t i) const { return items[i].val; }

	inline in_segment_t const *lookup(str_t key) const {
		item_t const *item = __lookup(key);
		return item ? &item->val : NULL;
	}

private: // don't use
	mime_header_t(mime_header_t const &);
	mime_header_t &operator=(mime_header_t const &);
};

class entity_t : public in_segment_list_t {
public:
	mime_header_t trailer;

	inline entity_t() :
		in_segment_list_t(), trailer() { }

	inline ~entity_t() throw() { }

	inline void clear() {
		in_segment_list_t::clear();
		trailer.clear();
	}

	bool parse(
		in_t::ptr_t &ptr, eol_t const &eol, mime_header_t const &header,
		limits_t const &limits, bool reply
	);

private: // don't use
	entity_t(entity_t const &);
	entity_t &operator=(entity_t const &);
};

version_t parse_version(in_t::ptr_t &p);

bool token_find(in_segment_t const &str, str_t token);

string_t time_current_string(timeval_t &timeval_cur);
string_t time_string(timeval_t time);
bool time_parse(in_segment_t const &str, timeval_t &time);
bool number_parse(in_segment_t const &str, size_t &num);

}} // namespace pd::http

#pragma GCC visibility pop
