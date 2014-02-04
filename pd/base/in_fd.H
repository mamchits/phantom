// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/in_buf.H>

#pragma GCC visibility push(default)

namespace pd {

class in_fd_t : public in_buf_t {
	virtual size_t readv(iovec *iov, size_t cnt);

	int fd;

public:
	inline in_fd_t(
		size_t _page_size, int _fd, log::level_t _log_level = log::error
	) throw() : in_buf_t(_page_size, _log_level), fd(_fd) { }

	~in_fd_t() throw();
};

} // namespace pd

#pragma GCC visibility pop
