// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/lock_guard.H>
#include <pd/base/assert.H>

#pragma GCC visibility push(default)

namespace pd {

#if defined(__x86_64__)

class spinlock_t {
	int val;

public:
	inline constexpr spinlock_t() throw() : val(1) { }

	inline void lock() throw() {
		while(__sync_sub_and_fetch(&val, 1))
			do { asm volatile ("rep; nop" ::: "memory"); } while(val <= 0);
	}

	inline void unlock() throw() {
		asm volatile ("" ::: "memory");
		val = 1;
		asm volatile ("" ::: "memory");
	}

	inline ~spinlock_t() throw() { assert(val == 1); }

	spinlock_t(spinlock_t const &) = delete;
	spinlock_t &operator=(spinlock_t const &) = delete;
};

#else

class spinlock_t {
	int val;

public:
	inline constexpr spinlock_t() throw() : val(0) { }

	inline void lock() throw() {
		while(__sync_lock_test_and_set(&val, 1) != 0)
			do { asm volatile ("" ::: "memory"); } while(val != 0);
	}

	inline void unlock() throw() { __sync_lock_release(&val); }

	inline ~spinlock_t() throw() { assert(!val); }

	spinlock_t(spinlock_t const &) = delete;
	spinlock_t &operator=(spinlock_t const &) = delete;
};

#endif

typedef lock_guard_t<spinlock_t> spinlock_guard_t;

} // namespace pd

#pragma GCC visibility pop
