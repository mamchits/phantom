// This file is part of the pd::pi library.
// Copyright (C) 2012-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "pi.H"

namespace pd { namespace pi {

static bool update_hex(char c, unsigned int *_v) {
	unsigned char _c;
	switch(c) {
		case '0'...'9': _c = c - '0'; break;
		case 'a'...'f': _c = c - 'a' + 10; break;
		case 'A'...'F': _c = c - 'A' + 10; break;
		default:
			return false;
	}

	(*_v *= 16) += _c;
	return true;
}

static str_t const str_null = CSTR("null");
static str_t const str_any = CSTR("any");
static str_t const str_false = CSTR("false");
static str_t const str_true = CSTR("true");

class enum_table_def_t : public pi_t::enum_table_t {
	virtual str_t lookup(unsigned int value) const;
	virtual unsigned int lookup(str_t const &str) const;

public:
	inline enum_table_def_t() throw() { }
	inline ~enum_table_def_t() throw() { }

	static enum_table_def_t const instance;
};

unsigned int enum_table_def_t::lookup(str_t const &_str) const {
	switch(_str.size()) {
		case 3: {
			if(str_t::cmp_eq<ident_t>(_str, str_any)) return pi_t::_any;
		}
		break;
		case 4: {
			if(str_t::cmp_eq<ident_t>(_str, str_null)) return pi_t::_null;
			if(str_t::cmp_eq<ident_t>(_str, str_true)) return pi_t::_true;
		}
		break;
		case 5: {
			if(str_t::cmp_eq<ident_t>(_str, str_false)) return pi_t::_false;
			{
				char const *str = _str.ptr();
				unsigned int _v = 0;
				if(
					(*str++) == '_' &&
					update_hex((*str++), &_v) && update_hex((*str++), &_v) &&
					update_hex((*str++), &_v) && update_hex((*str++), &_v)
				)
					return pi_t::_private | _v;
			}
		}
		break;
	}

	return ~0U;
}

static __thread char enum_lookup_def_buf[sizeof("_xxyy") - 1];

str_t enum_table_def_t::lookup(unsigned int value) const {
	switch(value) {
		case pi_t::_null: return str_null;
		case pi_t::_any: return str_any;
		case pi_t::_false: return str_false;
		case pi_t::_true: return str_true;
	}

	if(value > pi_t::_private) {
		unsigned int _v = value & ~pi_t::_private;
		char *ptr = enum_lookup_def_buf;

		*(ptr++) = '_';
		*(ptr++) = "0123456789abcdef"[_v >> 12];
		*(ptr++) = "0123456789abcdef"[(_v >> 8) & 0xf];
		*(ptr++) = "0123456789abcdef"[(_v >> 4) & 0xf];
		*(ptr++) = "0123456789abcdef"[_v & 0xf];

		return str_t(enum_lookup_def_buf, sizeof(enum_lookup_def_buf));
	}

	return str_t(NULL, 0);
}

enum_table_def_t const enum_table_def_t::instance;

} // namespace pi

pi_t::enum_table_t const &pi_t::enum_table_def = pi::enum_table_def_t::instance;

} // namespace pd
