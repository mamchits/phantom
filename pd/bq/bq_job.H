// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/bq/bq_thr.H>

#include <pd/base/thr.H>
#include <pd/base/log.H>

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <pd/base/fbind.H>
#endif

#pragma GCC visibility push(default)

namespace pd {

struct bq_job_base_t {
	string_t const name;
	log::backend_t const &log_backend;

	virtual void _run() const = 0;
	virtual ~bq_job_base_t() throw();

protected:
	void __create(bq_thr_t *bq_thr);
	void __create(pthread_t *pthread, pthread_attr_t *attr);

	inline bq_job_base_t(string_t const &_name) :
		name(_name.copy()), log_backend(log::handler_t::get_backend()) { }
};

template<typename func_t>
class bq_job_t;

#ifdef __GXX_EXPERIMENTAL_CXX0X__

template<typename x_t, typename ...args_t>
class bq_job_t<void (x_t::*)(args_t ...)> : bq_job_base_t {
	typedef void (x_t::*mfunc_t)(args_t ...);

	fbind_t<mfunc_t> fbind;

	virtual void _run() const { fbind(); }

	inline bq_job_t(
		string_t const &_name, x_t &_x, mfunc_t _mfunc, args_t ...args
	) :
		bq_job_base_t(_name), fbind(_x, _mfunc, args...) { }

public:
	inline static void create(
		string_t const &name, bq_thr_t *bq_thr,
		x_t &_x, mfunc_t _mfunc, args_t ...args
	) {
		(new bq_job_t(name, _x, _mfunc, args...))->__create(bq_thr);
	}

	inline static void create(
		string_t const &name, pthread_t *pthred, pthread_attr_t *attr,
		x_t &_x, mfunc_t _mfunc, args_t ...args
	) {
		(new bq_job_t(name, _x, _mfunc, args...))->__create(pthred, attr);
	}
};

template<typename x_t, typename ...args_t>
class bq_job_t<void (x_t::*)(args_t ...) const> : bq_job_base_t {
	typedef void (x_t::*mfunc_t)(args_t ...) const;

	fbind_t<mfunc_t> fbind;

	virtual void _run() const { fbind(); }

	inline bq_job_t(
		string_t const &_name, x_t const &_x, mfunc_t _mfunc, args_t ...args
	) :
		bq_job_base_t(_name), fbind(_x, _mfunc, args...) { }

public:
	inline static void create(
		string_t const &name, bq_thr_t *bq_thr,
		x_t const &_x, mfunc_t _mfunc, args_t ...args
	) {
		(new bq_job_t(name, _x, _mfunc, args...))->__create(bq_thr);
	}

	inline static void create(
		string_t const &name, pthread_t *pthred, pthread_attr_t *attr,
		x_t &_x, mfunc_t _mfunc, args_t ...args
	) {
		(new bq_job_t(name, _x, _mfunc, args...))->__create(pthred, attr);
	}
};

#else // !__GXX_EXPERIMENTAL_CXX0X__

// 0
template<typename x_t>
class bq_job_t<void (x_t::*)()> : bq_job_base_t {
	typedef void (x_t::*mfunc_t)();

	x_t &x;
	mfunc_t mfunc;

	virtual void _run() const { (x.*mfunc)(); }

	inline bq_job_t(
		string_t const &_name, x_t &_x, mfunc_t _mfunc
	) :
			bq_job_base_t(_name), x(_x), mfunc(_mfunc) { }

public:
	inline static void create(
		string_t const &name, bq_thr_t *bq_thr,
		x_t &_x, mfunc_t _mfunc
	) {
		(new bq_job_t(name, _x, _mfunc))->__create(bq_thr);
	}

	inline static void create(
		string_t const &name, pthread_t *pthred, pthread_attr_t *attr,
		x_t &_x, mfunc_t _mfunc
	) {
		(new bq_job_t(name, _x, _mfunc))->__create(pthred, attr);
	}
};

// 1
template<typename x_t, typename x1_t>
class bq_job_t<void (x_t::*)(x1_t x1)> : bq_job_base_t {
	typedef void (x_t::*mfunc_t)(x1_t x1);

	x_t &x;
	mfunc_t mfunc;
	x1_t x1;

	virtual void _run() const { (x.*mfunc)(x1); }

	inline bq_job_t(
		string_t const &_name, x_t &_x, mfunc_t _mfunc, x1_t _x1
	) :
			bq_job_base_t(_name), x(_x), mfunc(_mfunc), x1(_x1) { }

public:
	inline static void create(
		string_t const &name, bq_thr_t *bq_thr,
		x_t &_x, mfunc_t _mfunc, x1_t _x1
	) {
		(new bq_job_t(name, _x, _mfunc, _x1))->__create(bq_thr);
	}

	inline static void create(
		string_t const &name, pthread_t *pthred, pthread_attr_t *attr,
		x_t &_x, mfunc_t _mfunc, x1_t _x1
	) {
		(new bq_job_t(name, _x, _mfunc, _x1))->__create(pthred, attr);
	}
};

// 1, const
template<typename x_t, typename x1_t>
class bq_job_t<void (x_t::*)(x1_t x1) const> : bq_job_base_t {
	typedef void (x_t::*mfunc_t)(x1_t x1) const;

	x_t const &x;
	mfunc_t mfunc;
	x1_t x1;

	virtual void _run() const { (x.*mfunc)(x1); }

	inline bq_job_t(
		string_t const &_name, x_t const &_x, mfunc_t _mfunc, x1_t _x1
	) :
			bq_job_base_t(_name), x(_x), mfunc(_mfunc), x1(_x1) { }

public:
	inline static void create(
		string_t const &name, bq_thr_t *bq_thr,
		x_t const &_x, mfunc_t _mfunc, x1_t _x1
	) {
		(new bq_job_t(name, _x, _mfunc, _x1))->__create(bq_thr);
	}

	inline static void create(
		string_t const &name, pthread_t *pthred, pthread_attr_t *attr,
		x_t const &_x, mfunc_t _mfunc, x1_t _x1
	) {
		(new bq_job_t(name, _x, _mfunc, _x1))->__create(pthred, attr);
	}
};

// 2
template<typename x_t, typename x1_t, typename x2_t>
class bq_job_t<void (x_t::*)(x1_t x1, x2_t x2)> : bq_job_base_t {
	typedef void (x_t::*mfunc_t)(x1_t x1, x2_t x2);

	x_t &x;
	mfunc_t mfunc;
	x1_t x1; x2_t x2;

	virtual void _run() const { (x.*mfunc)(x1, x2); }

	inline bq_job_t(
		string_t const &_name, x_t &_x, mfunc_t _mfunc, x1_t _x1, x2_t _x2
	) :
			bq_job_base_t(_name), x(_x), mfunc(_mfunc), x1(_x1), x2(_x2) { }

public:
	inline static void create(
		string_t const &name, bq_thr_t *bq_thr,
		x_t &_x, mfunc_t _mfunc, x1_t _x1, x2_t _x2
	) {
		(new bq_job_t(name, _x, _mfunc, _x1, _x2))->__create(bq_thr);
	}

	inline static void create(
		string_t const &name, pthread_t *pthred, pthread_attr_t *attr,
		x_t &_x, mfunc_t _mfunc, x1_t _x1, x2_t _x2
	) {
		(new bq_job_t(name, _x, _mfunc, _x1, _x2))->__create(pthred, attr);
	}
};

// 2, const
template<typename x_t, typename x1_t, typename x2_t>
class bq_job_t<void (x_t::*)(x1_t x1, x2_t x2) const> : bq_job_base_t {
	typedef void (x_t::*mfunc_t)(x1_t x1, x2_t x2) const;

	x_t const &x;
	mfunc_t mfunc;
	x1_t x1; x2_t x2;

	virtual void _run() const { (x.*mfunc)(x1, x2); }

	inline bq_job_t(
		string_t const &_name, x_t const &_x, mfunc_t _mfunc, x1_t _x1, x2_t _x2
	) :
			bq_job_base_t(_name), x(_x), mfunc(_mfunc), x1(_x1), x2(_x2) { }

public:
	inline static void create(
		string_t const &name, bq_thr_t *bq_thr,
		x_t const &_x, mfunc_t _mfunc, x1_t _x1, x2_t _x2
	) {
		(new bq_job_t(name, _x, _mfunc, _x1, _x2))->__create(bq_thr);
	}

	inline static void create(
		string_t const &name, pthread_t *pthred, pthread_attr_t *attr,
		x_t const &_x, mfunc_t _mfunc, x1_t _x1, x2_t _x2
	) {
		(new bq_job_t(name, _x, _mfunc, _x1, _x2))->__create(pthred, attr);
	}
};

#endif

} // namespace pd

#pragma GCC visibility pop
