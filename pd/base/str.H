// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/op.H>
#include <pd/base/cmp.H>

#pragma GCC visibility push(default)

namespace pd {

class str_t {
	char const *data;
	size_t len;

public:
	inline str_t() throw() : data(NULL), len(0) { }

	inline str_t(char const *_data, size_t _len) throw() :
		data(_data), len(_len) { }

	inline char const *ptr() const throw() { return data; }
	inline size_t size() const throw() { return len; }

	inline str_t &truncate(size_t _len) {
		minimize(len, _len);

		return *this;
	}

	inline operator bool() const { return len > 0; }

	template<typename type_t>
	static inline cmp_t cmp(str_t const &str1, str_t const &str2) {
		size_t s1 = str1.len, s2 = str2.len;
		size_t len = min(s1, s2);
		cmp_t res = type_t::cmp(str1.data, str2.data, len);
		return res ? (cmp_t)(s1 - s2) : res;
	}

	template<typename type_t>
	static inline bool cmp_eq(str_t const &str1, str_t const &str2) {
		size_t s1 = str1.len, s2 = str2.len;
		if(s1 != s2) return false;
		return type_t::cmp(str1.data, str2.data, s1);
	}

	class ptr_t: public ptr_ops_t<ptr_t> {
		char const *ptr, *bound;

	public:
		inline ptr_t(str_t const &str) throw() :
			ptr(str.data), bound(str.data + str.len) { }

		inline char const *operator()() const throw() { return ptr; }
		inline char operator*() const throw() { return *ptr; }

		inline ptr_t &operator+=(size_t _len) {
			ptr += _len;
			return *this;
		}

		inline size_t size() const { return bound - ptr; }
		inline operator bool() const { return ptr < bound; }
	};

	template<typename type_t>
	inline uint64_t fnv() const {
		fnv_t h;
		ptr_t ptr = *this;
		while(ptr) h(type_t::map(*(ptr++)));
		return h;
	}
};

extern char const *illegal_cstr;

#define CSTR(s) \
	str_t( \
		(__builtin_constant_p(s) ? s : illegal_cstr), \
		(__builtin_constant_p(s) ? sizeof(s) - 1 : 0) \
	)

} // namespace pd

#pragma GCC visibility pop
