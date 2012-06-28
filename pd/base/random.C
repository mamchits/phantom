// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "random.H"
#include "exception.H"

#include <stdlib.h>

namespace pd {

static __thread bool inited = false;
static __thread struct drand48_data data;

inline void random_check() {
	if(!inited) {
		srand48_r((uintptr_t)&data, &data);
		inited = true;
	}
}

double random_F() {
	double res;

	random_check();

	if(drand48_r(&data, &res) < 0)
		throw exception_sys_t(log::error, errno, "drand48_r: %m");

	return res;
}

unsigned int random_U() {
	long int res;

	random_check();

	if(lrand48_r(&data, &res) < 0)
		throw exception_sys_t(log::error, errno, "lrand48_r: %m");

	return res;
}

int random_D() {
	long int res;

	random_check();

	if(mrand48_r(&data, &res) < 0)
		throw exception_sys_t(log::error, errno, "mrand48_r: %m");

	return (int)res;
}

} // namespace pd
