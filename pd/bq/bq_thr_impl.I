// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include "bq_thr.H"
#include "bq_heap.H"

#include <pd/base/thr.H>
#include <pd/base/string.H>

namespace pd {

class bq_thr_t::impl_t {
	typedef thr::mutex_t mutex_t;
	typedef thr::mutex_guard_t mutex_guard_t;

	bq_cont_count_t &cont_count;
	bq_cont_activate_t &activate;
	mutex_t mutex;
	string_t const name;
	pthread_t thread;
	pid_t tid;

	size_t maxevs;
	interval_t timeout;

	static bool work;

	struct stat_t;
	stat_t *stat, *stat_sum;

	timeval_t time;

	int efd;
	int sig_fds[2];

	bq_heap_t common, ready;

	inline void insert(bq_heap_t::item_t *item, bool is_ready) {
		(is_ready ? ready : common).insert(item);
	}

	inline bool remove(bq_heap_t::item_t *item) {
		bq_heap_t *heap = item->heap;

		heap->remove(item);

		return heap == &ready;
	}

	impl_t(
		string_t const &_name, size_t _maxevs, interval_t _timeout,
		bq_cont_count_t &_cont_count, bq_cont_activate_t &_activate
	);

	~impl_t() throw();

	void loop();
	void stat_print(out_t &out, bool clear);

	static __thread impl_t *current;

public:
	class item_t : public bq_heap_t::item_t {
	protected:
		impl_t *impl;

	private:
		virtual void attach() throw() = 0;
		virtual void detach() throw() = 0;

	protected:
		inline item_t(interval_t *_timeout) :
			bq_heap_t::item_t(_timeout), impl(bq_thr_get()->impl) { }

	public:
		inline bq_err_t suspend(bool is_ready, char const *where) {
			impl->accept(this, is_ready);
			return bq_cont_deactivate(where, is_ready ? wait_ready : wait_event);
		}

		inline impl_t *set_ready() {
			return impl->set_ready(this) ? impl : NULL;
		}
	};

	inline bool set_ready(bq_heap_t::item_t *item) {
		mutex_guard_t guard(mutex);

		if(item->heap == &ready)
			return false;

		common.remove(item);
		ready.insert(item);
		return true;
	}

	inline void accept(bq_heap_t::item_t *item, bool is_ready) {
		mutex_guard_t guard(mutex);
		insert(item, is_ready);
		item->attach();
	}

	void poke() const throw();

private:
	friend class poll_item_t;
	friend class bq_thr_t;
};

bq_err_t bq_do_poll(
	int fd, short int &events, interval_t *timeout, char const *where
);

} // namespace pd
