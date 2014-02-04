// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <stddef.h>

#pragma GCC visibility push(default)

namespace pd {

template<typename x_t>
inline constexpr x_t const &min(x_t const &a, x_t const &b) {
	return b < a ? b : a;
}

template<typename x_t, typename y_t>
inline x_t &minimize(x_t &x, y_t const &a) {
	if((x_t)a < x) x = a;
	return x;
}

template<typename x_t>
inline constexpr x_t const &max(x_t const &a, x_t const &b) {
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

template<typename x_t, typename base_t>
class dim_t : rel_ops_t<x_t> {
protected:
	base_t _val;

public:
	inline explicit constexpr dim_t(base_t __val) throw() : _val(__val) { }

	inline constexpr dim_t() throw() : _val(0) { }
	inline constexpr dim_t(dim_t const &) = default;
	inline dim_t &operator=(dim_t const &) = default;
	inline ~dim_t() = default;

	inline base_t constexpr val() const { return _val; }

	friend inline constexpr x_t operator+(x_t const &i) throw() {
		return x_t(+i._val);
	}

	friend inline constexpr x_t operator-(x_t const &i) throw() {
		return x_t(-i._val);
	}

	friend inline x_t &operator+=(x_t &x0, x_t const &i) throw() {
		x0._val += i._val; return x0;
	}

	friend inline constexpr x_t operator+(x_t const &i1, x_t const &i2) throw() {
		return x_t(i1._val + i2._val);
	}

	friend inline x_t &operator-=(x_t &x0, x_t const &i) throw() {
		x0._val -= i._val; return x0;
	}

	friend inline constexpr x_t operator-(x_t const &i1, x_t const &i2) throw() {
		return x_t(i1._val - i2._val);
	}

	friend inline x_t &operator*=(x_t &x0, base_t const &x) throw() {
		x0._val *= x; return x0;
	}

	friend inline constexpr x_t operator*(x_t const &i, base_t const &x) throw() {
		return x_t((base_t)(i._val * x));
	}

	friend inline constexpr x_t operator*(base_t const &x, x_t const &i) throw() {
		return x_t((base_t)(x * i._val));
	}

	friend inline x_t &operator/=(x_t &x0, base_t const &x) throw() {
		x0._val /= x; return x0;
	}

	friend inline constexpr x_t operator/(x_t const &i, base_t const &x) throw() {
		return x_t((base_t)(i._val / x));
	}

	friend inline constexpr base_t operator/(x_t const &i1, x_t const &i2) throw() {
		return i1._val / i2._val;
	}

	friend inline x_t &operator%=(x_t &i0, x_t const &i) throw() {
		i0._val %= i._val; return i0;
	}

	friend inline constexpr x_t operator%(x_t const &i1, x_t const &i2) throw() {
		return x_t(i1._val % i2._val);
	}

	friend inline constexpr bool operator==(x_t const &i1, x_t const &i2) throw() {
		return i1._val == i2._val;
	}

	friend inline constexpr bool operator<(x_t const &i1, x_t const &i2) throw() {
		return i1._val < i2._val;
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
