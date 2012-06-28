// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/config_helper.H>

#pragma GCC visibility push(default)

namespace pd {

template<typename x_t>
struct frac_t {
	x_t numerator, denominator;

	inline frac_t(x_t const &_numerator, x_t const &_denominator) throw() :
		numerator(_numerator), denominator(_denominator) { }

	inline ~frac_t() throw() { }

	template<typename y_t>
	friend inline y_t operator*(y_t const &y, frac_t const &fr) throw() {
		return (y * fr.numerator) / fr.denominator;
	}

	template<typename y_t>
	friend inline y_t operator/(y_t const &y, frac_t const &fr) throw() {
		return (y * fr.denominator) / fr.numerator;
	}

	template<typename y_t>
	friend inline y_t &operator*=(y_t &y, frac_t const &fr) throw() {
		return y = (y * fr);
	}

	template<typename y_t>
	friend inline y_t &operator/=(y_t &y, frac_t const &fr) throw() {
		return y = (y / fr);
	}
};

namespace config {

template<typename x_t>
struct helper_t<frac_t<x_t> > {
	static void parse(in_t::ptr_t &ptr, frac_t<x_t> &val) {
		frac_t<x_t> _val;
		in_t::ptr_t p = ptr;

		helper_t<x_t>::parse(p, _val.numerator);

		if(!p.match<ident_t>('/'))
			error(p, "'/' is expected");

		helper_t<x_t>::parse(p, _val.denominator);

		val = _val;
		ptr = p;
	}

	static void print(out_t &out, int off, frac_t<x_t> const &val) {
		helper_t<x_t>::print(out, off, val.numerator);
		out('/');
		helper_t<x_t>::print(out, off, val.denominator);
	}
};

} // namespace config

} // namespace pd

#pragma GCC visibility pop
