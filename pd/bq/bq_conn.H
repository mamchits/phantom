// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/time.H>
#include <pd/base/log.H>

#pragma GCC visibility push(default)

namespace pd {

class bq_conn_t {
public:
	log::level_t log_level;

	virtual void setup_connect() = 0;
	virtual void setup_accept() = 0;
	virtual void shutdown() = 0;

	virtual size_t readv(iovec *iov, size_t cnt, interval_t *timeout) = 0;
	virtual size_t writev(iovec const *iov, size_t cnt, interval_t *timeout) = 0;
	virtual void sendfile(int file_fd, off_t &offset, size_t &size, interval_t *timeout) = 0;

	virtual void wait_read(interval_t *timeout) = 0;

	virtual void ctl(int i) = 0;

	inline bq_conn_t(log::level_t _log_level) throw() : log_level(_log_level) { }
	virtual ~bq_conn_t() throw();
};

} // namespace pd

#pragma GCC visibility pop
