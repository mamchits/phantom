// This file is part of the pd::base library.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#pragma GCC visibility push(default)

namespace pd {

template<typename x_t>
struct opt_item_t {
	x_t x;

	inline constexpr opt_item_t(x_t const &_x) : x(_x) { }

	inline constexpr opt_item_t(opt_item_t const &) = default;
	inline opt_item_t &operator=(opt_item_t const &) = default;

	inline ~opt_item_t() throw() { }

	inline operator x_t const &() const throw() { return x; }
};

template<typename x_t, x_t x>
struct opt_val_t {
	typedef x_t val_t;
	static constexpr x_t def_val() { return x; }
};


template<typename x_t>
struct opt_null_t {
	typedef x_t val_t;
	static constexpr x_t def_val() { return nullptr; }
};


template<typename ...args_t>
class opt_items_t : public opt_item_t<typename args_t::val_t>... {
	template<typename x_t>
	struct assign_item_t {
		inline assign_item_t(opt_items_t &opts, x_t const &x) {
			opts.opt_item_t<x_t>::x = x;
		}
	};

	template<typename ..._args_t>
	struct assign_t : assign_item_t<_args_t>... {
		inline assign_t(opt_items_t &opts, _args_t const &..._args) :
			assign_item_t<_args_t>(opts, _args)... { }
	};

public:
	template<typename ..._args_t>
	inline opt_items_t(_args_t const &..._args) :
		opt_item_t<typename args_t::val_t>(args_t::def_val())... {

		assign_t<_args_t...> _assign(*this, _args...);
	}

	inline constexpr opt_items_t(opt_items_t const &) = default;
	inline opt_items_t &operator=(opt_items_t const &) = default;

	inline ~opt_items_t() throw() { }
};

} // namespace pd

#pragma GCC visibility pop
