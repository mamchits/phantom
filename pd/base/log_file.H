// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/string.H>
#include <pd/base/stat.H>

#pragma GCC visibility push(default)

namespace pd {

namespace log_file {
class __hidden handler_t;
}

class log_file_t {
	typedef log_file::handler_t handler_t;

	handler_t &handler;

public:
	struct guard_t {
		handler_t  &handler;
		bool used;
		int const fd;
		stat::count_t &ocount;

		guard_t(log_file_t const &log_file) throw();
		~guard_t() throw();
	};

	friend struct guard_t;

	log_file_t(string_t const &filename, string_t const &_header);
	~log_file_t() throw();

	void _init();
	void _stat_print() const;
	void check() const;

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
