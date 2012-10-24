// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/bq/bq.H>

#include <pd/base/thr.H>
#include <pd/base/assert.H>

#pragma GCC visibility push(default)

namespace pd {

class bq_mutex_t {
	thr::spinlock_t spinlock;

	bool state;

	class __hidden wait_item_t;
	wait_item_t *list, **last;

public:
	inline bq_mutex_t() throw() :
		spinlock(), state(false), list(NULL), last(&list) { }

	inline ~bq_mutex_t() throw() {
		assert(state == false);
		assert(list == NULL);
	}

	bq_err_t lock(interval_t *timeout) throw();
	void lock();
	void unlock() throw();

	friend class wait_item_t;
};

typedef thr::guard_t<bq_mutex_t> bq_mutex_guard_t;

} // namespace pd

#pragma GCC visibility pop
