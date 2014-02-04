// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "in_fd.H"
#include "exception.H"

namespace pd {

size_t in_fd_t::readv(iovec *iov, size_t cnt) {
	ssize_t res = ::readv(fd, iov, cnt);

	if(res < 0)
		throw exception_sys_t(get_log_level(), errno, "readv: %m");

	return res;
}

in_fd_t::~in_fd_t() throw() { }

} // namespace pd
