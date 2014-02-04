// This file is part of the pd::base library.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/lock_guard.H>
#include <pd/base/mutex.H>

#pragma GCC visibility push(default)

namespace pd {

namespace cond {

class queue_t {
	class __hidden item_t;
	item_t *list;
	item_t **last;

public:
	inline queue_t() throw() : list(NULL), last(&list) { }
	inline ~queue_t() throw() { assert(list == NULL); }

	int wait(mutex_t &mutex, interval_t *timeout) throw();
	void wait(mutex_t &mutex) throw();
	void send(bool broadcast) throw();

	friend class item_t;
};

enum single_t { single = 0 };

} // namespace cond

template<unsigned int n, typename enum_t>
class cond_base_t {
	mutex_t mutex;

	cond::queue_t queue[n];

public:
	inline cond_base_t() throw() : mutex() { }
	inline ~cond_base_t () throw() { }

	cond_base_t(cond_base_t const &) = delete;
	cond_base_t &operator=(cond_base_t const &) = delete;

	inline void lock() throw() { mutex.lock(); }
	inline void unlock() throw() { mutex.unlock(); }

	typedef lock_guard_t<cond_base_t> guard_t;

	static inline constexpr unsigned int fix(enum_t idx) {
		return (unsigned int)idx < n ? (unsigned int)idx : 0;
	}

	class handler_t {
		cond_base_t &cond;

	public:
		inline handler_t(cond_base_t &_cond) : cond(_cond) { cond.lock(); }

		inline ~handler_t() throw() { cond.unlock(); }

		inline int wait(enum_t idx, interval_t *timeout) {
			return cond.wait(idx, timeout);
		}

		inline void wait(enum_t idx) { cond.wait(idx); }

		inline void send(enum_t idx, bool broadcast = false) throw() {
			cond.send(idx, broadcast);
		}
	};

private:
	inline int wait(enum_t idx, interval_t *timeout) throw() {
		return queue[fix(idx)].wait(mutex, timeout);
	}

	inline void wait(enum_t idx) {
		queue[fix(idx)].wait(mutex);
	}

	inline void send(enum_t idx, bool broadcast) throw() {
		queue[fix(idx)].send(broadcast);
	}

	friend class handler_t;
};

class cond_t : public cond_base_t<1, cond::single_t> {
public:
	inline cond_t() : cond_base_t<1, cond::single_t>() { }

	inline ~cond_t() throw() { }

	class handler_t : public cond_base_t<1, cond::single_t>::handler_t {
	public:
		inline handler_t(cond_t &_cond) :
			cond_base_t<1, cond::single_t>::handler_t(_cond) { }

		inline ~handler_t() throw() { }

		inline int wait(interval_t *timeout) {
			return cond_base_t<1, cond::single_t>::handler_t::wait(cond::single, timeout);
		}

		inline void wait() {
			cond_base_t<1, cond::single_t>::handler_t::wait(cond::single);
		}

		inline void send(bool broadcast = false) throw() {
			cond_base_t<1, cond::single_t>::handler_t::send(cond::single, broadcast);
		}
	};
};

} // namespace pd

#pragma GCC visibility pop
