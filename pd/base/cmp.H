// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <string.h>
#include <stdint.h>

#pragma GCC visibility push(default)

namespace pd {

class cmp_t {
	int val;
public:
	inline cmp_t(int _val) throw() : val(_val) { }
	inline ~cmp_t() throw() { }

	inline operator bool() const throw() { return val == 0; }
	inline bool is_less() const throw() { return val < 0; }
	inline bool is_greater() const throw() { return val > 0; }
};

struct ident_t {
	static inline char map(char c) { return c; }

	static inline cmp_t cmp(char const *ptr1, char const *ptr2, size_t len) {
		return memcmp(ptr1, ptr2, len);
	}
};

struct lower_t {
	static inline char map(char c) {
		return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
	}

	static inline cmp_t cmp(char const *ptr1, char const *ptr2, size_t len) {
		while(len--) {
			int c = map(*(ptr1++)) - map(*(ptr2++));
			if(c) return c;
		}
		return 0;
	}
};

class fnv_t {
	uint64_t n;

public:
	inline fnv_t() throw() : n(0xcbf29ce484222325ULL) { }

	inline fnv_t &operator()(unsigned char c) {
		(n ^= c) *= 0x100000001b3ULL;
		return *this;
	}

	inline operator uint64_t() const { return n; }

	inline ~fnv_t() throw() { }
};

} // namespace pd

#pragma GCC visibility pop
