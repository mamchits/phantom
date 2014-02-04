// This file is part of the pd::base library.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "cond.H"
#include "list.H"

#include "futex.I"

namespace pd {

namespace cond {

class queue_t::item_t : public list_item_t<item_t> {
	queue_t &queue;
	int val;
	bool broadcast;

	inline item_t(queue_t &_queue) throw() :
		list_item_t<item_t>(this, *_queue.last), queue(_queue),
		val(1), broadcast(false) {

		queue.last = &next;
	}

	inline ~item_t() throw() {
		assert(!val);

		if(queue.last == &next)
			queue.last = me;

		if(next && next->broadcast)
			next->wake();
	}

	inline void wait() {
		futex_wait(&val, 1);
	}

	inline void wake() {
		val = 0;
		futex_wake(&val);
	}

	friend class queue_t;
};

int queue_t::wait(mutex_t &, interval_t *) throw() {
	fatal("not implemented"); // FIXME
}

void queue_t::wait(mutex_t &mutex) throw() {
	item_t item(*this);
	mutex.unlock();
	item.wait();
	mutex.lock();
}

void queue_t::send(bool broadcast) throw() {
	if(list) {
		if(broadcast) {
			for(item_t *item = list->next; item; item = item->next)
				item->broadcast = true;
		}

		list->wake();
	}
}

} // namespace cond

} // namespace pd
