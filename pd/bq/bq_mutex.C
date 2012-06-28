// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_mutex.H"
#include "bq_thr_impl.I"

#include <pd/base/exception.H>

namespace pd {

class bq_mutex_t::wait_item_t : public bq_thr_t::impl_t::item_t {
	bq_mutex_t &mutex;
	wait_item_t *next, **me;

	virtual void attach() throw();
	virtual void detach() throw();

public:
	inline wait_item_t(interval_t *timeout, bq_mutex_t &_mutex) :
		item_t(timeout), mutex(_mutex) { }

	inline ~wait_item_t() throw() { }

	friend class bq_mutex_t;
};

void bq_mutex_t::wait_item_t::attach() throw() {
	*(me = mutex.last) = this;
	*(mutex.last = &next) = NULL;

	mutex.spinlock.unlock();
}

void bq_mutex_t::wait_item_t::detach() throw() {
	mutex.spinlock.lock();

	if((*me = next)) next->me = me;

	if(mutex.last == &next)
		mutex.last = me;
}

bq_err_t bq_mutex_t::lock(interval_t *timeout) {
	thr::spinlock_guard_t guard(spinlock);

	if(!state) {
		state = 1;
		return bq_ok;
	}

	if(timeout && *timeout == interval_zero)
		return bq_timeout;

	wait_item_t item(timeout, *this);

	return item.suspend(false, "mutex-lock");
}

void bq_mutex_t::unlock() {
	bq_thr_t::impl_t *impl = ({
		thr::spinlock_guard_t guard(spinlock);

		if(!list) {
			state = 0;
			return;
		}

		list->set_ready();
	});

	if(impl) impl->poke();
}

void bq_mutex_t::lock() {
	if(!bq_success(lock(NULL)))
		throw exception_sys_t(log::error, errno, "mutex-lock: %m");
}

} // namespace pd
