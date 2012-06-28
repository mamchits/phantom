// This file is part of the phantom::io_benchmark module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/out.H>
#include <pd/base/thr.H>
#include <pd/base/time.H>
#include <pd/base/size.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_benchmark {

class descr_t {
public:
	virtual size_t value_max() const = 0;
	virtual void print_header(out_t &out) const = 0;
	virtual void print_value(out_t &out, size_t value) const = 0;

	inline descr_t() throw() { }
	inline ~descr_t() throw() { }

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

class times_t : public descr_t {
	virtual size_t value_max() const;
	virtual void print_header(out_t &out) const;
	virtual void print_value(out_t &out, size_t value) const;

protected:
	size_t size;
	interval_t *steps;

	inline times_t(size_t _size) : size(_size) { steps = new interval_t[size]; }

	inline ~times_t() throw() { delete[] steps; }

public:
	size_t index(interval_t interval) const;
};

class method_t;

class stat_t {
	method_t const &method;
	times_t const &times;
	bool hrr_flag;

	class res_t;
	res_t *res, *res_sum;

	size_t times_ind;

	struct tcount_t {
		thr::spinlock_t spinlock;
		size_t val, val_max, val_max_total;

		inline tcount_t() throw() : spinlock(), val(0), val_max(0), val_max_total(0) { }

		inline void inc() {
			thr::spinlock_guard_t guard(spinlock);
			if(++val > val_max) val_max = val;
		}

		inline void dec() {
			thr::spinlock_guard_t guard(spinlock);
			--val;
		}

		inline size_t get(bool clear) {
			thr::spinlock_guard_t guard(spinlock);
			if(val_max > val_max_total) val_max_total = val_max;
			size_t res = clear ? val_max : val_max_total;
			val_max = val;
			return res;
		}
	} tcount;

public:
	thr::spinlock_t spinlock;

	stat_t(method_t const &_method, times_t const &_times, bool _hrr_flag);
	~stat_t() throw();

	class tcount_guard_t {
		tcount_t &tcount;
	public:
		inline tcount_guard_t(stat_t &stat) : tcount(stat.tcount) { tcount.inc(); }
		inline ~tcount_guard_t() throw() { tcount.dec(); }
	};

	friend class tcount_guard_t;

	void update(size_t index, size_t value);
	void update_time(interval_t time_real, interval_t time_events);
	void update_size(sizeval_t size_in, sizeval_t size_out);
	void event();
	void print(out_t &out, bool clear);
};

class method_t {
public:
	virtual bool test(stat_t &stat) const = 0;
	virtual void init() = 0;
	virtual void stat(out_t &out, bool clear, bool hrr_flag) const = 0;
	virtual void fini() = 0;

	virtual size_t maxi() const throw() = 0;
	virtual descr_t const *descr(size_t ind) const throw() = 0;

protected:
	inline method_t() throw() { }
	inline ~method_t() throw() { }

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

}} // namespace phantom::io_benchmark

#pragma GCC visibility pop
