// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_cond.H"
#include "bq_thr_impl.I"

#include <pd/base/exception.H>

namespace pd {

namespace bq_cond {

void bq_cond::lock_t::lock() { spinlock.lock(); }

void bq_cond::lock_t::unlock() {
	bq_thr_t::impl_t *_impl = thr_impl;
	thr_impl = NULL;
	spinlock.unlock();
	if(_impl) _impl->poke();
}

class queue_t::wait_item_t : public bq_thr_t::impl_t::item_t {
	queue_t &queue;
	lock_t &lock;
	bool broadcast;
	wait_item_t *next, **me;

	virtual void attach() throw();
	virtual void detach() throw();

public:
	inline wait_item_t(interval_t *timeout, queue_t &_queue, lock_t &_lock) :
		item_t(timeout, false), queue(_queue), lock(_lock), broadcast(false) { }

	inline ~wait_item_t() throw() { }

	friend class queue_t;
};

void queue_t::wait_item_t::attach() throw() {
	*(me = queue.last) = this;
	*(queue.last = &next) = NULL;

	lock.unlock();
}

void queue_t::wait_item_t::detach() throw() {
	lock.lock();

	if((*me = next)) next->me = me;

	if(queue.last == &next)
		queue.last = me;

	if(next && next->broadcast)
		lock.thr_impl = next->set_ready();
}

bq_err_t queue_t::wait(lock_t &lock, interval_t *timeout) throw() {
	wait_item_t item(timeout, *this, lock);

	return item.suspend("cond-wait");
}

void queue_t::wait(lock_t &lock) {
	if(!bq_success(wait(lock, NULL)))
		throw exception_sys_t(log::error, errno, "cond-wait: %m");
}

void queue_t::send(lock_t &lock, bool broadcast) throw() {
	if(list) {
		if(broadcast) {
			for(wait_item_t *item = list->next; item; item = item->next)
				item->broadcast = true;
		}

		lock.thr_impl = list->set_ready();
	}
}

} // namespace bq_cond

} // namespace pd
