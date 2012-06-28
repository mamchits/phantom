// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/config.H>

#pragma GCC visibility push(default)

namespace pd { namespace config {

template<typename val_t>
class struct_t : public val_t, public block_t {
public:

#ifdef __GXX_EXPERIMENTAL_CXX0X__

	template<typename ...args_t>
	inline struct_t(args_t const &...args) : val_t(args...), block_t() { }

#else

	inline struct_t() : val_t(), block_t() { }

	template<typename x1_t, typename x2_t, typename x3_t, typename x4_t>
	inline struct_t(
		x1_t const &x1, x2_t const &x2, x3_t const &x3, x4_t const &x4
	) : val_t(x1, x2, x3, x4), block_t() { }

#endif

	inline ~struct_t() throw() { }

private:
	virtual void parse_content(in_t::ptr_t &ptr) {
		binding_t<val_t>::parse(ptr, *this);
	}

	virtual void print_content(out_t &out, int off) const {
		binding_t<val_t>::print(out, off, *this);
	}
};

template<typename val_t>
struct helper_t<struct_t<val_t> > {
	static void parse(in_t::ptr_t &ptr, struct_t<val_t> &val) {
		val.block_t::parse(ptr);
	}

	static void print(out_t &out, int off, struct_t<val_t> const &val) {
		val.block_t::print(out, off);
	}

	static void syntax(out_t &out) {
		out('{')(' ')('[')(binding_t<val_t>::sname)(' ')(']')('*')('}');
		syntax_t::push(binding_t<val_t>::sname, &binding_t<val_t>::syntax);
	}
};

}} // namespace pd::config

#pragma GCC visibility pop
