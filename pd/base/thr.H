// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/assert.H>

#include <pthread.h>

#pragma GCC visibility push(default)

namespace pd { namespace thr {

template<typename lock_t>
class guard_t {
	lock_t &lock;
	bool active;

public:
	inline void wakeup() { lock.lock(); active = true; }
	inline void relax() { lock.unlock(); active = false; }

	inline guard_t(lock_t &_lock) : lock(_lock) { wakeup(); }
	inline ~guard_t() { if(active) relax(); }

private: // don't use
	guard_t(guard_t const &);
	guard_t &operator=(guard_t const &);
};

#if 0 && (defined(__i386__) || defined(__x86_64__))

class spinlock_t {
	union cnts_t {
		unsigned int raw;
		struct {
			unsigned short lo, hi;
		};
	} cnts;

public:
	inline spinlock_t() throw() { cnts.raw = 0; }

	inline void lock() throw() {
		cnts_t ocnts; ocnts.raw = __sync_fetch_and_add(&cnts.raw, 0x10000);
		unsigned short hi = ocnts.hi;
		unsigned short lo = ocnts.lo;

		while(hi != lo) {
			asm volatile ("rep; nop" ::: "memory");
			lo = cnts.lo;
		}
	}

	inline void unlock() throw() {
#ifdef __i386__
		__sync_fetch_and_add(&cnts.lo, 1);
#else
		asm volatile ("" ::: "memory");
		++cnts.lo;
#endif
	}

	inline ~spinlock_t() throw() {
		cnts_t ocnts = cnts;
		asm volatile ("" ::: "memory");
		assert(ocnts.lo == ocnts.hi);
	}

private: // don't use
	spinlock_t(spinlock_t const &);
	spinlock_t &operator=(spinlock_t const &);
};

#else

class spinlock_t {
	pthread_spinlock_t pthread_spin;

public:
	inline spinlock_t() throw() {
		pthread_spin_init(&pthread_spin, 0);
	}

	inline void lock() throw() {
		pthread_spin_lock(&pthread_spin);
	}

	inline void unlock() throw() {
		pthread_spin_unlock(&pthread_spin);
	}

	inline ~spinlock_t() throw() {
		pthread_spin_destroy(&pthread_spin);
	}

private: // don't use
	spinlock_t(spinlock_t const &);
	spinlock_t &operator=(spinlock_t const &);
};

#endif

typedef guard_t<spinlock_t> spinlock_guard_t;

class mutex_t {
protected:
	pthread_mutex_t pthread_mutex;

public:
	inline mutex_t() throw() {
		pthread_mutex_init(&pthread_mutex, NULL);
	}

	inline void lock() throw() {
		pthread_mutex_lock(&pthread_mutex);
	}

	inline void unlock() throw() {
		pthread_mutex_unlock(&pthread_mutex);
	}

	inline ~mutex_t() throw() {
		pthread_mutex_destroy(&pthread_mutex);
	}

private: // don't use
	mutex_t(mutex_t const &);
	mutex_t &operator=(mutex_t const &);
};

typedef guard_t<mutex_t> mutex_guard_t;

}} // namespace pd::thr

#pragma GCC visibility pop
