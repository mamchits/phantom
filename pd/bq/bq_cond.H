// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/bq/bq_thr.H>

#pragma GCC visibility push(default)

namespace pd {

class bq_cond_t {
	thr::spinlock_t spinlock;

	class __hidden wait_item_t;
	wait_item_t *list;
	wait_item_t **last;

	bq_thr_t::impl_t *impl;

public:
	inline bq_cond_t() throw() :
		spinlock(), list(NULL), last(&list), impl(NULL) { }

	inline ~bq_cond_t() throw() {
		assert(list == NULL);
		assert(impl == NULL);
	}

	void lock() throw();
	void unlock() throw();

	void send(bool broadcast = false) throw();
	bq_err_t wait(interval_t *timeout) throw();

	friend class wait_item_t;
};

typedef thr::guard_t<bq_cond_t> bq_cond_guard_t;

} // namespace pd

#pragma GCC visibility pop
