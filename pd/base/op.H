// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <stddef.h>

#pragma GCC visibility push(default)

namespace pd {

template<typename x_t>
inline x_t const &min(x_t const &a, x_t const &b) {
	return b < a ? b : a;
}

template<typename x_t, typename y_t>
inline x_t &minimize(x_t &x, y_t const &a) {
	if((x_t)a < x) x = a;
	return x;
}

template<typename x_t>
inline x_t const &max(x_t const &a, x_t const &b) {
	return a < b ? b : a;
}

template<typename x_t, typename y_t>
inline x_t &maximize(x_t &x, y_t const &a) {
	if(x < (x_t)a) x = a;
	return x;
}

template<typename x_t>
class rel_ops_t {
	friend inline bool operator!=(x_t const &x1, x_t const &x2) {
		return !(x1 == x2);
	}

	friend inline bool operator>(x_t const &x1, x_t const &x2) {
		return (x2 < x1);
	}

	friend inline bool operator<=(x_t const &x1, x_t const &x2) {
		return !(x2 < x1);
	}

	friend inline bool operator>=(x_t const &x1, x_t const &x2) {
		return !(x1 < x2);
	}
};

template<typename x_t>
class ptr_ops_t : public rel_ops_t<x_t> {
	friend inline x_t &operator++(x_t &x) { return x += 1U; }

	friend inline x_t operator++(x_t &x, int) {
		x_t _x(x);
		x += 1U;
		return _x;
	}

	friend inline x_t operator+(x_t &x, size_t n) {
		x_t _x(x); return (_x += n);
	}
};

} // namespace pd

#pragma GCC visibility pop
