// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "string.H"
#include "config_helper.H"
#include "log.H"

namespace pd {

class string_t::page_t : public in_t::page_t {
	size_t size;
	char data[0];

	inline page_t(size_t _size) throw() : in_t::page_t(), size(_size) { }

	inline void *operator new(size_t size, size_t data_size) {
		return ::operator new(size + data_size);
	}

	virtual bool chunk(size_t off, str_t &str) const;
	virtual unsigned int depth() const;
	virtual bool optimize(in_segment_t &segment) const;

	friend class ctor_t;
};

bool string_t::page_t::chunk(size_t off, str_t &str) const {
	if(off >= size)
		return false;

	str = str_t(data + off, size - off);

	return true;
}

unsigned int string_t::page_t::depth() const { return 0; }

bool string_t::page_t::optimize(in_segment_t &) const { return false; }

void string_t::ctor_t::setup_out(size_t _size, size_t _wpos) {
	data = string_page->data;
	rpos = size = _size;
	wpos = _wpos;
}

string_t::ctor_t::ctor_t(size_t _size) : out_t(NULL, 0) {
	string_page = new(_size) page_t(_size);

	setup_out(_size, 0);
}

string_t::ctor_t::~ctor_t() throw() {
	if(string_page)
		delete string_page;
}

void string_t::ctor_t::rollback(char c) {
	while(wpos && string_page->data[--wpos] != c);
}

string_t::string_t(ctor_t &ctor) {
	list = ctor.string_page;
	off_begin = 0;
	off_end = ctor.wpos;
	ctor.string_page = NULL;
}


class static_page_t : public in_t::page_t {
	virtual bool chunk(size_t off, str_t &str) const;
	unsigned int depth() const;
	virtual bool optimize(in_segment_t &segment) const;

public:
	inline static_page_t() : in_t::page_t() { }

	virtual ~static_page_t() throw();


	inline void *operator new(size_t size);
	inline void operator delete(void *) { }
};

static char static_page_buf[sizeof(static_page_t)] __aligned(__alignof__(static_page_t));

inline void *static_page_t::operator new(size_t size) {
	assert(size == sizeof(static_page_t));
	return static_page_buf;
}

bool static_page_t::chunk(size_t off, str_t &str) const {
	str = str_t((char const *)NULL + off, (size_t)(-1) - off);

	return true;
}

unsigned int static_page_t::depth() const { return 0; }

bool static_page_t::optimize(in_segment_t &) const { return false; }

static_page_t::~static_page_t() throw() { }

void string_t::ctor_t::flush() {
	log_warning("string buffer overflow");

	size_t _size = max((2 * size), (size_t)1024);
	page_t *_string_page = new(_size) page_t(_size);
	memcpy(_string_page->data, string_page->data, size);
	delete string_page;
	string_page = _string_page;

	setup_out(_size, size);
}

ref_t<in_t::page_t> __init_priority(101) string_t::static_page_ref = new static_page_t;

string_t __init_priority(101) string_t::empty;

template<>
void out_t::helper_t<string_t>::print(
	out_t &out, string_t const &string, char const *fmt
) {
	out.print(string.str(), fmt);
}

namespace config {

template<>
void helper_t<string_t>::parse(in_t::ptr_t &ptr, string_t &string) {
	in_t::ptr_t p = ptr;
	if(*p !=  '"')
		error(p, "'\"' is expected");

	ptr = ++p;

	while(true) {
		switch(*p) {
			case '"':
				string = string_t(ptr, p - ptr).copy();
				ptr = ++p;
				return;
			case '\r': case '\n': case '\0':
				error(ptr, "unterminated string");
			case '\\':
				goto long_path;
			default:
				++p;
		}
	}

long_path:
	in_t::ptr_t px = p;

	size_t len = 0;
	while(true) {
		switch(*px) {
			case '\\':
				switch(*(++px)) {
					case '\\': case '"': case 'r': case 'n': case '0': break;
					default:
						error(px, "unknown escape char");
				}
			break;
			case '\r': case '\n': case '\0':
				error(px, "unterminated string");
			case '"':
				goto final;
		}
		++px;
		++len;
	}

final:
	string_t::ctor_t ctor(p - ptr + len);
	ctor(string_t(ptr, p - ptr));

	while(true) {
		char c;
		switch(c = *(p++)) {
			case '\\':
				switch(c = *(p++)) {
					case '\\': case '"': break;
					case 'r': c = '\r'; break;
					case 'n': c = '\n'; break;
					case '0': c = '\0'; break;
					default:
						error(p, "unknown escape char");
				}
			break;
		case '"':
			string = ctor;
			ptr = p;
			return;
		}

		ctor(c);
	}
}

template<>
void helper_t<string_t>::print(out_t &out, int, string_t const &string) {
	char const *p = string.ptr();
	char const *bound = p + string.size();

	out('"');

	while(p < bound) {
		char c;
		switch(c = *(p++)) {
			case '\r': c = 'r'; break;
			case '\n': c = 'n'; break;
			case '\0': c = '0'; break;
			case '"': case '\\': break;
			default:
				goto noesc;

		}
		out('\\');
noesc:
		out(c);
	}

	out('"');
}

template<>
void helper_t<string_t>::syntax(out_t &out) {
	out(CSTR("<string>"));
}

} // namespace config

} // namespace pd
