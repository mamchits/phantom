// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_cond.H"
#include "bq_thr_impl.I"

namespace pd {

class bq_cond_t::wait_item_t : public bq_thr_t::impl_t::item_t {
	bq_cond_t &cond;
	wait_item_t *next, **me;

	virtual void attach() throw();
	virtual void detach() throw();

public:
	inline wait_item_t(interval_t *timeout, bq_cond_t &_cond) :
		item_t(timeout), cond(_cond) { }

	inline ~wait_item_t() throw() { }

	friend class bq_cond_t;
};

void bq_cond_t::wait_item_t::attach() throw() {
	*(me = cond.last) = this;
	*(cond.last = &next) = NULL;

	cond.unlock();
}

void bq_cond_t::wait_item_t::detach() throw() {
	cond.lock();

	if((*me = next)) next->me = me;

	if(cond.last == &next)
		cond.last = me;
}

void bq_cond_t::lock() {
	spinlock.lock();
}

void bq_cond_t::unlock() {
	bq_thr_t::impl_t *_impl = impl;
	impl = NULL;

	spinlock.unlock();

	if(_impl) _impl->poke();
}

bq_err_t bq_cond_t::wait(interval_t *timeout) {
	wait_item_t item(timeout, *this);

	return item.suspend(false, "cond-wait");
}

void bq_cond_t::send() {
	if(list)
		impl = list->set_ready();
}

} // namespace pd
