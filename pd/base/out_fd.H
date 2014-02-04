// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/out.H>
#include <pd/base/fd.H>
#include <pd/base/log.H>
#include <pd/base/stat.H>
#include <pd/base/opt.H>

#pragma GCC visibility push(default)

namespace pd {

class out_fd_t : public out_t {
	int fd;
	fd_ctl_t const *fd_ctl;
	log::level_t log_level;
	stat::count_t *stat;

	virtual void flush();
	virtual out_t &ctl(int i);
	virtual out_t &sendfile(int fd, off_t &offset, size_t &size);

	typedef opt_items_t<
		opt_null_t<fd_ctl_t const *>,
		opt_val_t<log::level_t, log::error>,
		opt_null_t<stat::count_t *>
	> opts_t;

public:
	template<typename ..._args_t>
	inline out_fd_t(char *_data, size_t _size, int _fd, _args_t const &..._args) :
		out_t(_data, _size), fd(_fd), fd_ctl(opts_t(_args...)),
		log_level(opts_t(_args...)), stat(opts_t(_args...)) { }

	~out_fd_t() throw();
};

} // namespace pd

#pragma GCC visibility pop
