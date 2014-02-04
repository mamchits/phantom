// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include "bq_thr.H"
#include "bq_heap.H"

#include <pd/base/spinlock.H>
#include <pd/base/string.H>
#include <pd/base/thr.H>
#include <pd/base/job.H>
#include <pd/base/stat.H>
#include <pd/base/stat_items.H>

namespace pd {

class bq_thr_t::impl_t {
	bq_cont_count_t &cont_count;
	bq_post_activate_t *post_activate;
	job_id_t thread;
	pid_t tid;

	size_t maxevs;
	interval_t timeout;

	static bool work;

	typedef stat::mmcount_t conts_t;
	typedef stat::count_t acts_t;

	typedef stat::items_t<
		conts_t,
		acts_t,
		thr::tstate_t
	> stat_base_t;

	struct stat_t : stat_base_t {
		inline stat_t() throw() : stat_base_t(
			STRING("mmconts"),
			STRING("acts"),
			STRING("tstate")
		) { }

		inline ~stat_t() throw() { }

		inline conts_t &conts() throw() { return item<0>(); }
		inline acts_t &acts() throw() { return item<1>(); }
		inline thr::tstate_t &tstate() throw() { return item<2>(); }
	};

	stat_t stat;

	inline void stat_print() {
		char buf[16];
		size_t len = ({
			out_t out(buf, sizeof(buf));
			out.print(tid, "05").used();
		});

		stat::ctx_t ctx(str_t(buf, len));
		stat.print();
	}

	int efd;
	int sig_fds[2];

	struct entry_t {
		spinlock_t spinlock;
		bq_heap_t::item_t *list;

		inline entry_t() throw() : spinlock(), list(NULL) { }
		inline ~entry_t() throw() { assert(!list); }

		inline void accept(bq_heap_t::item_t *item) {
			spinlock_guard_t guard(spinlock);

			if(item->me) return;

			if((item->next = list)) list->me = &item->next;
			item->me = &list; list = item;
		}

		inline bool set_ready(bq_heap_t::item_t *item) {
			spinlock_guard_t guard(spinlock);
			if(item->ready)
				return false;

			item->ready = true;

			if(item->me)
				return true;

			if((item->next = list)) list->me = &item->next;
			item->me = &list; list = item;

			return true;
		}

		inline void remove(bq_heap_t::item_t *item) {
			spinlock_guard_t guard(spinlock);
			if(!item->me)
				return;

			if((*item->me = item->next))
				item->next->me = item->me;

			item->me = NULL;
		}

		inline bq_heap_t::item_t *remove() {
			spinlock_guard_t guard(spinlock);
			if(!list)
				return NULL;

			bq_heap_t::item_t *item = list;

			if((list = list->next))
				list->me = &list;

			item->me = NULL;

			return item;
		}
	};

	entry_t entry;

	impl_t(
		size_t _maxevs, interval_t _timeout, bq_cont_count_t &_cont_count,
		bq_post_activate_t *_post_activate
	);

	~impl_t() throw();

	void loop();
	void init(string_t const &tname);
	void fini();

	static __thread impl_t *current;

public:
	class item_t : public bq_heap_t::item_t {
	protected:
		impl_t *impl;

	private:
		virtual void attach() throw() = 0;
		virtual void detach() throw() = 0;

	protected:
		inline item_t(interval_t *_timeout, bool ready) :
			bq_heap_t::item_t(_timeout, ready),
			impl(({
				bq_thr_t *bq_thr = bq_thr_get();
				assert(bq_thr);
				bq_thr->impl;
			})) { }

	public:
		inline bq_err_t suspend(char const *where) {
			impl->entry.accept(this);

			attach();
			bq_cont_deactivate(where);
			detach();

			impl->entry.remove(this); // (!)

			if(impl->post_activate)
				(*impl->post_activate)(this);

			return err;
		}

		inline impl_t *set_ready() {
			return impl->entry.set_ready(this) ? impl : NULL;
		}
	};

	void poke() const throw();

	friend class poll_item_t;
	friend class bq_thr_t;
};

} // namespace pd
