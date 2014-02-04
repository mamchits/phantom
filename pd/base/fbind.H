// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/vtempl.H>
#include <pd/base/ref.H>

#pragma GCC visibility push(default)

namespace pd {

namespace fbind {

template<int i, typename x_t>
struct item_t {
	x_t x;
	inline item_t(x_t _x) : x(_x) { }
};

template<typename idxs_t, typename ...args_t>
struct __items_t;

template<int ...idxs, typename ...args_t>
struct __items_t<vtempl::ints_t<idxs...>, args_t...> : item_t<idxs, args_t>... {
	inline __items_t(args_t ...args) : item_t<idxs, args_t>(args)... { }
};

template<typename ...args_t>
struct items_t :
	__items_t<typename vtempl::idxs_t<sizeof ...(args_t)>::type_t, args_t...> {

	typedef typename vtempl::idxs_t<sizeof ...(args_t)>::type_t idxs_t;

	inline items_t(args_t ..._args) : __items_t<idxs_t, args_t...>(_args...) { }
};

} // namespace fbind

template<typename>
class fbind_t;

template<typename res_t, typename ...args_t>
class fbind_t<res_t (*)(args_t ...args)> {
	typedef res_t (*func_t)(args_t ...args);

	func_t func;
	fbind::items_t<args_t...> args;

	template<typename>
	struct _t;

	template<int ...idxs>
	struct _t<vtempl::ints_t<idxs...>> {
		static inline res_t apply(fbind_t const &fbind) {
			return (*fbind.func)(fbind.args.fbind::item_t<idxs, args_t>::x...);
		}
	};

public:
	inline fbind_t(func_t _func, args_t ..._args) :
		func(_func), args(_args...) { }

	inline res_t operator()() const {
		return _t<typename fbind::items_t<args_t...>::idxs_t>::apply(*this);
	}
};

template<typename res_t, typename x_t, typename ...args_t>
class fbind_t<res_t (x_t::*)(args_t ...)> {
	typedef res_t (x_t::*mfunc_t)(args_t ...);

	x_t &x;
	mfunc_t mfunc;
	fbind::items_t<args_t...> args;

	template<typename>
	struct _t;

	template<int ...idxs>
	struct _t<vtempl::ints_t<idxs...>> {
		static inline res_t apply(fbind_t const &fbind) {
			return (fbind.x.*fbind.mfunc)(fbind.args.fbind::item_t<idxs, args_t>::x...);
		}
	};

public:
	inline fbind_t(x_t &_x, mfunc_t _mfunc, args_t ..._args) :
		x(_x), mfunc(_mfunc), args(_args...) { }

	inline res_t operator()() const {
		_t<typename fbind::items_t<args_t...>::idxs_t>::apply(*this);
	}
};

template<typename res_t, typename x_t, typename ...args_t>
class fbind_t<res_t (x_t::*)(args_t ...) const> {
	typedef res_t (x_t::*mfunc_t)(args_t ...) const;

	x_t const &x;
	mfunc_t mfunc;
	fbind::items_t<args_t...> args;

	template<typename>
	struct _t;

	template<int ...idxs>
	struct _t<vtempl::ints_t<idxs...>> {
		static inline res_t apply(fbind_t const &fbind) {
			return (fbind.x.*fbind.mfunc)(fbind.args.fbind::item_t<idxs, args_t>::x...);
		}
	};

public:
	inline fbind_t(x_t const &_x, mfunc_t _mfunc, args_t ..._args) :
		x(_x), mfunc(_mfunc), args(_args...) { }

	inline res_t operator()() const {
		return _t<typename fbind::items_t<args_t...>::idxs_t>::apply(*this);
	}
};

// -------------------------------------------------

template<typename, typename>
class job_t;

template<typename res_t, typename ...args_t, typename base_t>
class job_t<res_t (*)(args_t ...), base_t> : public base_t {
	typedef res_t (*func_t)(args_t ...);

	fbind_t<func_t> fbind;

	virtual res_t _run() const { return fbind(); }
	virtual ~job_t() throw() { }

public:
	inline job_t(func_t _func, args_t ...args) :
		base_t(), fbind(_func, args...) { }
};

template<typename res_t, typename x_t, typename ...args_t, typename base_t>
class job_t<res_t (x_t::*)(args_t ...), base_t> : public base_t {
	typedef res_t (x_t::*mfunc_t)(args_t ...);

	fbind_t<mfunc_t> fbind;

	virtual res_t _run() const { return fbind(); }
	virtual ~job_t() throw() { }

public:
	inline job_t(x_t &_x, mfunc_t _mfunc, args_t ...args) :
		base_t(), fbind(_x, _mfunc, args...) { }
};

template<typename res_t, typename x_t, typename ...args_t, typename base_t>
class job_t<res_t (x_t::*)(args_t ...) const, base_t> : public base_t {
	typedef res_t (x_t::*mfunc_t)(args_t ...) const;

	fbind_t<mfunc_t> fbind;

	virtual res_t _run() const { return fbind(); }
	virtual ~job_t() throw() { }

public:
	inline job_t(x_t const &_x, mfunc_t _mfunc, args_t ...args) :
		base_t(), fbind(_x, _mfunc, args...) { }
};

// -------------------------------------------------

template<typename, typename>
class job_gen_t;

template<typename res_t, typename ...args_t, typename base_t>
class job_gen_t<res_t (*)(args_t ...), base_t> {
	typedef res_t (*func_t)(args_t ...);

	func_t func;

public:
	inline job_gen_t(func_t _func) : func(_func) { }

	inline ~job_gen_t() throw() { }

	inline job_gen_t(job_gen_t const &) = default;
	inline job_gen_t &operator=(job_gen_t const &) = default;

	inline base_t *operator()(args_t ...args) {
		return new job_t<func_t, base_t>(func, args...);
	}
};

template<typename res_t, typename x_t, typename ...args_t, typename base_t>
class job_gen_t<res_t (x_t::*)(args_t ...), base_t> {
	typedef res_t (x_t::*mfunc_t)(args_t ...);

	mfunc_t mfunc;

public:
	inline job_gen_t(mfunc_t _mfunc) : mfunc(_mfunc) { }

	inline ~job_gen_t() throw() { }

	inline job_gen_t(job_gen_t const &) = default;
	inline job_gen_t &operator=(job_gen_t const &) = default;

	inline ref_t<base_t> operator()(x_t &x, args_t ...args) {
		return ref_t<base_t>(new job_t<mfunc_t, base_t>(x, mfunc, args...));
	}
};

template<typename res_t, typename x_t, typename ...args_t, typename base_t>
class job_gen_t<res_t (x_t::*)(args_t ...) const, base_t> {
	typedef res_t (x_t::*mfunc_t)(args_t ...) const;

	mfunc_t mfunc;

public:
	inline job_gen_t(mfunc_t _mfunc) : mfunc(_mfunc) { }

	inline ~job_gen_t() throw() { }

	inline job_gen_t(job_gen_t const &) = default;
	inline job_gen_t &operator=(job_gen_t const &) = default;

	inline ref_t<base_t> operator()(x_t const &x, args_t ...args) {
		return ref_t<base_t>(new job_t<mfunc_t, base_t>(x, mfunc, args...));
	}
};

} // namespace pd

#pragma GCC visibility pop
