// This file is part of the pd::ssl library.
// Copyright (C) 2011, 2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011, 2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/ssl/ssl.H>

#include <pd/bq/bq_conn.H>

#include <pd/base/log.H>
#include <pd/base/exception.H>
#include <pd/base/time.H>
#include <pd/base/fd.H>

#pragma GCC visibility push(default)

namespace pd {

class bq_conn_ssl_t : public bq_conn_t {
	void *internal;

	interval_t timeout; // means SSL_{connect,accept,shutdown} timeout only
	int fd;
	fd_ctl_t const *fd_ctl;

	virtual void setup_connect();
	virtual void setup_accept();
	virtual void shutdown();

	virtual size_t readv(iovec *iov, size_t cnt, interval_t *timeout);
	virtual size_t writev(iovec const *iov, size_t cnt, interval_t *timeout);
	virtual void sendfile(int file_fd, off_t &offset, size_t &size, interval_t *timeout);
	virtual void ctl(int i);
	virtual void wait_read(interval_t *timeout);

public:
	bq_conn_ssl_t(
		int fd, ssl_ctx_t const &ctx, interval_t _timeout,
		fd_ctl_t const *_fd_ctl = NULL, log::level_t _log_level = log::error
	);

	virtual ~bq_conn_ssl_t() throw();
};

} // namespace pd

#pragma GCC visibility pop
