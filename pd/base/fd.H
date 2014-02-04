// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#pragma GCC visibility push(default)

#include <sys/types.h>

namespace pd {

class fd_ctl_t {
public:
	virtual void operator()(int fd, int i) const = 0;

	inline fd_ctl_t() throw() { }
	inline ~fd_ctl_t() throw() { }

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

} // namespace pd

#pragma GCC visibility pop
