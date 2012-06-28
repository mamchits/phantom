// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <stdint.h>

#pragma GCC visibility push(default)

namespace pd {

#ifdef _LP64

typedef unsigned int uint128_t __attribute__((mode(TI)));

#else

// Minimal set for ipv6 address manipulations

struct uint128_t {
	uint64_t lo, hi;

	inline uint128_t(uint64_t _lo = 0, uint64_t _hi = 0) :
		lo(_lo), hi(_hi) { }
};

inline uint128_t operator>>(uint128_t const &u, unsigned char i) {
	return i < 64
		? i
			? uint128_t((u.lo >> i) + (u.hi << (64 - i)), u.hi >> i)
			: uint128_t(u.lo, u.hi)
		: i < 128
			? uint128_t(u.hi >> (i - 64), 0)
			: uint128_t(0, 0)
	;
}

inline uint128_t operator<<(uint128_t const &u, unsigned char i) {
	return i < 64
		? i
			? uint128_t(u.lo << i, (u.hi << i) + (u.lo >> (64 - i)))
			: uint128_t(u.lo, u.hi)
		: i < 128
			? uint128_t(0, u.lo << (i - 64))
			: uint128_t(0, 0)
	;
}

inline uint128_t &operator<<=(uint128_t &u, unsigned char i) {
	u = (u << i);
	return u;
}

inline uint128_t operator^(uint128_t const &u1, uint128_t const &u2) {
	return uint128_t(u1.lo ^ u2.lo, u1.hi ^ u2.hi);
}

inline uint128_t operator&(uint128_t const &u1, uint128_t const &u2) {
	return uint128_t(u1.lo & u2.lo, u1.hi & u2.hi);
}

inline uint64_t operator&(uint128_t const &u1, uint64_t const &u2) {
	return u1.lo & u2;
}

inline uint128_t operator|(uint128_t const &u1, uint128_t const &u2) {
	return uint128_t(u1.lo | u2.lo, u1.hi | u2.hi);
}

inline bool operator==(uint128_t const &u1, uint128_t const &u2) {
	return u1.hi == u2.hi && u1.lo == u2.lo;
}

inline bool operator!=(uint128_t const &u1, uint128_t const &u2) {
	return !(u1 == u2);
}

#endif // !_LP64

} // namespace pd

#pragma GCC visibility pop
