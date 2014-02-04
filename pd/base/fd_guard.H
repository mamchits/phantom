// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/log.H>

#include <unistd.h>

#pragma GCC visibility push(default)

namespace pd {

class fd_guard_t {
protected:
	int fd;
public:
	inline fd_guard_t(int _fd) throw() : fd(_fd) { }
	inline void relax() { fd = -1; }
	inline ~fd_guard_t() throw() {
		if(fd >= 0)
			if(::close(fd) < 0)
				log_error("fd_guard_t::~fd_guard_t, close: %m");
	}
};

} // namespace pd

#pragma GCC visibility pop
