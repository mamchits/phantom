// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/bq/bq.H>

#include <pd/base/defs.H>
#include <pd/base/thr.H>
#include <pd/base/assert.H>

#include <sys/types.h>
#include <stddef.h>

#pragma GCC visibility push(default)

namespace pd {

class string_t;
class interval_t;
class out_t;
class bq_cont_activate_t;

class bq_cont_count_t {
	thr::spinlock_t spin;
	unsigned int counter;
	unsigned int limit;

public:
	inline bq_cont_count_t(unsigned int _limit) throw() :
		spin(), counter(0), limit(_limit) { }

	inline ~bq_cont_count_t() throw() { assert(!counter); }

	inline bool inc() {
		thr::spinlock_guard_t guard(spin);
		if(counter >= limit)
			return false;

		++counter;
		return true;
	}

	inline void dec() {
		thr::spinlock_guard_t guard(spin);
		--counter;
	}

	inline unsigned int value() {
		thr::spinlock_guard_t guard(spin);
		return counter;
	}

private: // don't use
	bq_cont_count_t(bq_cont_count_t const &);
	bq_cont_count_t &operator=(bq_cont_count_t const &);
};

class bq_thr_t {
public:
	class __hidden impl_t;

private:
	impl_t *impl;

public:
	inline bq_thr_t() throw() : impl(NULL) { }

	void init(
		string_t const &name, size_t _maxevs, interval_t _timeout,
		bq_cont_count_t &cont_count, bq_cont_activate_t &cont_activate
	);

	void fini();

	inline ~bq_thr_t() throw() { assert(impl == NULL); }

	bq_err_t switch_to(interval_t const &prio, bool force = false);

	bq_cont_count_t &cont_count() throw();

	void stat_print(out_t &out, bool clear);
	string_t const &name() const throw();
	pid_t get_tid() const throw();

	static void stop();
};

bq_err_t bq_cont_create(bq_thr_t *bq_thr, void (*fun)(void *), void *arg);

bq_thr_t *bq_thr_get() throw();
bool bq_thr_set(bq_thr_t *bq_thr) throw();

} // namespace pd

#pragma GCC visibility pop
