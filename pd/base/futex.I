// This file is part of the pd::base library.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <unistd.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <linux/futex.h>

namespace pd {

/*
inline int futex_lock_pi(int *futex) {
	return syscall(SYS_futex, futex, FUTEX_LOCK_PI_PRIVATE, 0, NULL, NULL, 0);
}

inline int futex_unlock_pi(int *futex) {
	return syscall(SYS_futex, futex, FUTEX_UNLOCK_PI_PRIVATE, 0, NULL, NULL, 0);
}
*/

inline int futex_wait(int *futex, int val) {
	return syscall(SYS_futex, futex, FUTEX_WAIT_PRIVATE, val, NULL, NULL, 0);
}

inline int futex_wake(int *futex) {
	return syscall(SYS_futex, futex, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
}

} // namespace pd
