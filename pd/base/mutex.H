// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/lock_guard.H>
#include <pd/base/stat.H>
#include <pd/base/stat_items.H>

#pragma GCC visibility push(default)

namespace pd {

class mutex_t {
	int val;
	pid_t tid;

	enum event_t { pass, wait };

	struct ecount_t : stat::vcount_t<event_t, 2> {
		inline ecount_t() :
			stat::vcount_t<event_t, 2> (
				STRING("pass"), STRING("wait")
			) { }
	};

	enum state_t { unlocked, locked };

	struct tstate_t : stat::tstate_t<state_t, 2> {
		inline tstate_t() :
			stat::tstate_t<state_t, 2> (
				STRING("unlocked"), STRING("locked")
			) { }
	};

	typedef stat::items_t<
		ecount_t,
		tstate_t
	> stat_base_t;

public:
	struct stat_t : stat_base_t {
		inline stat_t() throw() : stat_base_t(
			STRING("events"),
			STRING("tstate")
		) { }

		inline ~stat_t() throw() { }

		inline ecount_t &events() { return item<0>(); }
		inline tstate_t &tstate() { return item<1>(); }
	};

	stat_t stat;

	inline mutex_t() throw() : val(0), tid(0), stat() { }

	void lock();
	void unlock();

	inline void init() { stat.init(); }

	inline void stat_print() { stat.print(); }

	inline ~mutex_t() throw() { assert(!tid); assert(!val); }

	mutex_t(mutex_t const &) = delete;
	mutex_t &operator=(mutex_t const &) = delete;
};

typedef lock_guard_t<mutex_t> mutex_guard_t;

} // namespace pd

#pragma GCC visibility pop
