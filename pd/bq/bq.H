// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/time.H>

#include <sys/poll.h>

#pragma GCC visibility push(default)

namespace pd {

enum bq_err_t {
	bq_ok = 0,
	bq_timeout = 1,
	bq_not_available = 2,
	bq_overload = 3,
	bq_illegal_call = -1
};

bool bq_success(bq_err_t res) throw();

bq_err_t bq_do_poll(
	int fd, short int &events, interval_t *timeout, char const *where
);

} // namespace pd

#pragma GCC visibility pop
