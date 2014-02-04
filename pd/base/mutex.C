// This file is part of the pd::base library.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "mutex.H"
#include "thr.H"

#include "futex.I"

namespace pd {

void mutex_t::lock() {
	int oval = __sync_val_compare_and_swap(&val, 0, 1);

	if(!oval) {
		++stat.events()[pass];
	}
	else {
		assert(oval == 1 || oval == 2);
		thr::tstate_t *tstate = thr::tstate;

		thr::state_t old_state;

		if(tstate)
			old_state = tstate->set(thr::locked);

		while(__sync_lock_test_and_set(&val, 2) != 0)
			futex_wait(&val, 2);

		if(tstate)
			tstate->set(old_state);

		++stat.events()[wait];
	}

	tid = thr::id;

	stat.tstate().set(locked);
}

void mutex_t::unlock() {
	stat.tstate().set(unlocked);

	assert(val != 0);

	assert(tid == thr::id);

	tid = 0;

	int oval = __sync_sub_and_fetch(&val, 1);

	if(oval) {
		assert(oval == 1);
		val = 0;
		__sync_synchronize();
		futex_wake(&val);
	}
}

} // namespace pd
