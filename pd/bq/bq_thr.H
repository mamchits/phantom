// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/bq/bq_err.H>
#include <pd/bq/bq_heap.H>

#include <pd/base/defs.H>
#include <pd/base/spinlock.H>
#include <pd/base/assert.H>
#include <pd/base/stat.H>

#include <sys/types.h>
#include <stddef.h>

#pragma GCC visibility push(default)

namespace pd {

class bq_cont_count_t {
	spinlock_t spinlock;
	unsigned int counter;
	unsigned int limit;

public:
	inline bq_cont_count_t(unsigned int _limit) throw() :
		spinlock(), counter(0), limit(_limit) { }

	inline ~bq_cont_count_t() throw() { assert(!counter); }

	inline bool inc() {
		spinlock_guard_t guard(spinlock);
		if(counter >= limit)
			return false;

		++counter;
		return true;
	}

	inline void dec() {
		spinlock_guard_t guard(spinlock);
		--counter;
	}

	inline unsigned int value() {
		spinlock_guard_t guard(spinlock);
		return counter;
	}

	bq_cont_count_t(bq_cont_count_t const &) = delete;
	bq_cont_count_t &operator=(bq_cont_count_t const &) = delete;
};

class bq_thr_t {
public:
	class __hidden impl_t;

private:
	impl_t *impl;

public:
	inline bq_thr_t() throw() : impl(NULL) { }

	void init(
		size_t _maxevs, interval_t _timeout,
		bq_cont_count_t &cont_count, string_t const &tname,
		bq_post_activate_t *post_activate = NULL
	);

	void fini();

	inline ~bq_thr_t() throw() { assert(impl == NULL); }

	bq_err_t switch_to(interval_t const &prio, bool force = false);

	bq_cont_count_t &cont_count() throw();

	stat::mmcount_t &stat_conts() throw();

	void stat_print();
	pid_t get_tid() const throw();

	static void stop();
};

bq_err_t bq_cont_create(bq_thr_t *bq_thr, void (*fun)(void *), void *arg);

bq_thr_t *bq_thr_get() throw();
bool bq_thr_set(bq_thr_t *bq_thr) throw();

} // namespace pd

#pragma GCC visibility pop
