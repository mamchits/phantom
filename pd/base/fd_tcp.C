// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "fd_tcp.H"
#include "exception.H"

#include <netinet/in.h>
#include <netinet/tcp.h>

namespace pd {

void fd_ctl_tcp_t::operator()(int fd, int i) const {
	if(cork) {
		if(setsockopt(fd, SOL_TCP, TCP_CORK, &i, sizeof(i)) < 0)
			throw exception_sys_t(log::error, errno, "setsockopt, TCP_CORK, %d: %m", i);
	}
}

} // namespace pd
