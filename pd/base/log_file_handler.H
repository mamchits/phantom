// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/thr.H>
#include <pd/base/string.H>

#pragma GCC visibility push(default)

namespace pd {

class log_file_handler_t {
	mutable thr::spinlock_t spinlock;
	string_t const filename_z;
	string_t const header;

	struct file_t;
	file_t *current, *old;

	int acquire() const throw();
	void release(int fd) const throw();

public:
	log_file_handler_t(string_t const &filename, string_t const &_header);
	~log_file_handler_t() throw();

	void check();

	ssize_t writev(iovec const *iov, size_t count) const throw();

	inline ssize_t write(char const *str, size_t len) const throw() {
		iovec iov = (iovec) { (void *)str, len };
		return writev(&iov, 1);
	}

	inline ssize_t write(string_t const &str) const throw() {
		return write(str.ptr(), str.size());
	}
};

} // namespace pd

#pragma GCC visibility pop
