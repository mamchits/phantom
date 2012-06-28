// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#pragma GCC visibility push(default)

namespace pd {

#ifdef __GXX_EXPERIMENTAL_CXX0X__

template<int i, typename x_t>
struct wrap_t {
	x_t x;
	inline wrap_t(x_t _x) : x(_x) { }
};

// -----------------------------------

template<int ...>
struct int_tuple_t { };

template<int i, typename int_tuple_t>
struct __idxs_t;

template<int ...res>
struct __idxs_t<0, int_tuple_t<res...>> {
	typedef int_tuple_t<res...> type_t;
};

template<int i, int ...res>
struct __idxs_t<i, int_tuple_t<res...>> {
	typedef typename __idxs_t<i - 1, int_tuple_t<i - 1, res...>>::type_t type_t;
};

template<int i>
struct idxs_t : __idxs_t<i, int_tuple_t<>> { };

// -----------------------------------

template<typename idxs_t, typename ...args_t>
struct __tuple_t;

template<int ...idxs, typename ...args_t>
struct __tuple_t<int_tuple_t<idxs...>, args_t...> : wrap_t<idxs, args_t>... {

	inline __tuple_t(args_t ...args) : wrap_t<idxs, args_t>(args)... { }
};

template<typename ...args_t>
struct tuple_t :
	__tuple_t<typename idxs_t<sizeof...(args_t)>::type_t, args_t...> {

	typedef typename idxs_t<sizeof...(args_t)>::type_t idxs_t;

	inline tuple_t(args_t ..._args) :
		__tuple_t<idxs_t, args_t...>(_args...) { }
};

// -----------------------------------

template<typename>
class fbind_t;

template<typename res_t, typename ...args_t>
class fbind_t<res_t (*)(args_t ...args)> {
	typedef res_t (*func_t)(args_t ...args);

	func_t func;
	tuple_t<args_t...> args;

	template<typename>
	struct _t;

	template<int ...idxs>
	struct _t<int_tuple_t<idxs...>> {
		static inline res_t apply(fbind_t const &fbind) {
			return (*fbind.func)(fbind.args.wrap_t<idxs, args_t>::x...);
		}
	};

public:
	inline fbind_t(func_t _func, args_t ..._args) :
		func(_func), args(_args...) { }

	inline res_t operator()() const {
		return _t<typename tuple_t<args_t...>::idxs_t>::apply(*this);
	}
};

template<typename res_t, typename x_t, typename ...args_t>
class fbind_t<res_t (x_t::*)(args_t ...)> {
	typedef res_t (x_t::*mfunc_t)(args_t ...);

	x_t &x;
	mfunc_t mfunc;
	tuple_t<args_t...> args;

	template<typename>
	struct _t;

	template<int ...idxs>
	struct _t<int_tuple_t<idxs...>> {
		static inline res_t apply(fbind_t const &fbind) {
			return (fbind.x.*fbind.mfunc)(fbind.args.wrap_t<idxs, args_t>::x...);
		}
	};

public:
	inline fbind_t(x_t &_x, mfunc_t _mfunc, args_t ..._args) :
		x(_x), mfunc(_mfunc), args(_args...) { }

	inline res_t operator()() const {
		_t<typename tuple_t<args_t...>::idxs_t>::apply(*this);
	}
};

template<typename res_t, typename x_t, typename ...args_t>
class fbind_t<res_t (x_t::*)(args_t ...) const> {
	typedef res_t (x_t::*mfunc_t)(args_t ...) const;

	x_t const &x;
	mfunc_t mfunc;
	tuple_t<args_t...> args;

	template<typename>
	struct _t;

	template<int ...idxs>
	struct _t<int_tuple_t<idxs...>> {
		static inline res_t apply(fbind_t const &fbind) {
			return (fbind.x.*fbind.mfunc)(fbind.args.wrap_t<idxs, args_t>::x...);
		}
	};

public:
	inline fbind_t(x_t const &_x, mfunc_t _mfunc, args_t ..._args) :
		x(_x), mfunc(_mfunc), args(_args...) { }

	inline res_t operator()() const {
		return _t<typename tuple_t<args_t...>::idxs_t>::apply(*this);
	}
};

#endif

} // namespace pd

#pragma GCC visibility pop
