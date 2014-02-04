// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/time.H>

#pragma GCC visibility push(default)

namespace pd {

template<typename lock_t>
class lock_guard_t {
	lock_t &lock;
	bool active;

public:
	inline void wakeup() { lock.lock(); active = true; }
	inline void relax() { lock.unlock(); active = false; }

	inline lock_guard_t(lock_t &_lock) : lock(_lock) { wakeup(); }
	inline ~lock_guard_t() { if(active) relax(); }

	lock_guard_t(lock_guard_t const &) = delete;
	lock_guard_t &operator=(lock_guard_t const &) = delete;
};

template<typename cond_t>
class cond_handler_t {
	cond_t &cond;

public:
	inline cond_handler_t(cond_t &_cond) : cond(_cond) { cond.lock(); }
	inline ~cond_handler_t() throw() { cond.unlock(); }

	inline auto wait(interval_t *timeout) -> typeof(cond.wait(timeout)) {
		return cond.wait(timeout);
	}

	inline void wait() { cond.wait(); }
	inline void send(bool broadcast = false) throw() { cond.send(broadcast); }

	cond_handler_t(cond_handler_t &cond_handler_t) = delete;
	cond_handler_t &operator=(cond_handler_t const &) = delete;
};

} // namespace pd

#pragma GCC visibility pop
