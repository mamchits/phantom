// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/out.H>
#include <pd/base/fd.H>
#include <pd/base/log.H>

#pragma GCC visibility push(default)

namespace pd {

class out_fd_t : public out_t {
	int fd;
	fd_ctl_t const *fd_ctl;
	log::level_t log_level;

	virtual void flush();
	virtual out_t &ctl(int i);
	virtual out_t &sendfile(int fd, off_t &offset, size_t &size);

public:
	inline out_fd_t(
		char *_data, size_t _size, int _fd, fd_ctl_t const *_fd_ctl = NULL,
		log::level_t _log_level = log::error
	) :
		out_t(_data, _size), fd(_fd), fd_ctl(_fd_ctl), log_level(_log_level) {
	}

	~out_fd_t() throw();
};

} // namespace pd

#pragma GCC visibility pop
