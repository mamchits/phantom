// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/bq/bq_thr.H>

#pragma GCC visibility push(default)

namespace pd {

namespace bq_cond {

class lock_t;

class queue_t {
	class __hidden wait_item_t;
	wait_item_t *list;
	wait_item_t **last;

public:
	inline queue_t() throw() : list(NULL), last(&list) { }
	inline ~queue_t() throw() { assert(list == NULL); }

	bq_err_t wait(lock_t &lock, interval_t *timeout) throw();
	void wait(lock_t &lock);
	void send(lock_t &loock, bool broadcast) throw();

	friend class wait_item_t;
	friend class lock_t;
};

class lock_t {
	spinlock_t spinlock;
	bq_thr_t::impl_t *thr_impl;

public:
	inline lock_t() throw() : spinlock(), thr_impl(NULL) { }
	inline ~lock_t() throw() { assert(thr_impl == NULL); }

	void lock();
	void unlock();

	friend class queue_t;
	friend class queue_t::wait_item_t;
};

enum single_t { single = 0 };

} // namespace bq_cond

template<unsigned int n, typename enum_t>
class bq_cond_base_t : public bq_cond::lock_t {
	bq_cond::queue_t queue[n];

public:
	inline bq_cond_base_t() throw() : bq_cond::lock_t() { }
	inline ~bq_cond_base_t() throw() { }

	bq_cond_base_t(bq_cond_base_t const &) = delete;
	bq_cond_base_t &operator=(bq_cond_base_t const &) = delete;

	typedef lock_guard_t<bq_cond_base_t> guard_t;

	static inline constexpr unsigned int fix(enum_t idx) {
		return (unsigned int)idx < n ? (unsigned int)idx : 0;
	}

	class handler_t {
		bq_cond_base_t &cond;

	public:
		inline handler_t(bq_cond_base_t &_cond) : cond(_cond) { cond.lock(); }

		inline ~handler_t() throw() { cond.unlock(); }

		inline bq_err_t wait(enum_t idx, interval_t *timeout) {
			return cond.wait(idx, timeout);
		}

		inline void wait(enum_t idx) { cond.wait(idx); }

		inline void send(enum_t idx, bool broadcast = false) throw() {
			cond.send(idx, broadcast);
		}
	};

private:
	inline bq_err_t wait(enum_t idx, interval_t *timeout) throw() {
		return queue[fix(idx)].wait(*this, timeout);
	}

	inline void wait(enum_t idx) {
		queue[fix(idx)].wait(*this);
	}

	inline void send(enum_t idx, bool broadcast) throw() {
		queue[fix(idx)].send(*this, broadcast);
	}
};

class bq_cond_t : public bq_cond_base_t<1, bq_cond::single_t> {
public:
	inline bq_cond_t() : bq_cond_base_t<1, bq_cond::single_t>() { }

	inline ~bq_cond_t() throw() { }

	class handler_t : public bq_cond_base_t<1, bq_cond::single_t>::handler_t {
	public:
		inline handler_t(bq_cond_t &_bq_cond) :
			bq_cond_base_t<1, bq_cond::single_t>::handler_t(_bq_cond) { }

		inline ~handler_t() throw() { }

		inline bq_err_t wait(interval_t *timeout) {
			return bq_cond_base_t<1, bq_cond::single_t>::handler_t::wait(bq_cond::single, timeout);
		}

		inline void wait() {
			bq_cond_base_t<1, bq_cond::single_t>::handler_t::wait(bq_cond::single);
		}

		inline void send(bool broadcast = false) throw() {
			bq_cond_base_t<1, bq_cond::single_t>::handler_t::send(bq_cond::single, broadcast);
		}
	};
};

} // namespace pd

#pragma GCC visibility pop
