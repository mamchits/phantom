// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_conn_fd.H"
#include "bq_util.H"

#include <pd/base/exception.H>

#include <unistd.h>

namespace pd {

bq_conn_fd_t::bq_conn_fd_t(
	int _fd, fd_ctl_t const *_fd_ctl, log::level_t _log_level, bool _dup
) :
	bq_conn_t(_log_level),
	dup(_dup), in_fd(_fd), out_fd(dup ? ::dup(in_fd) : in_fd), fd_ctl(_fd_ctl) {

	if(out_fd < 0)
		throw exception_sys_t(log::error, errno, "dup: %m");
}

void bq_conn_fd_t::setup_accept() { }

void bq_conn_fd_t::setup_connect() { }

void bq_conn_fd_t::shutdown() { ::shutdown(in_fd, 2); }

size_t bq_conn_fd_t::readv(iovec *iov, size_t cnt, interval_t *timeout) {
	ssize_t res = bq_readv(in_fd, iov, cnt, timeout);

	if(res < 0)
		throw exception_sys_t(log_level, errno, "readv: %m");

	return res;
}

size_t bq_conn_fd_t::writev(iovec const *iov, size_t cnt, interval_t *timeout) {
	ssize_t res = bq_writev(out_fd, iov, cnt, timeout);

	if(res < 0)
		throw exception_sys_t(log_level, errno, "writev: %m");

	return res;
}

void bq_conn_fd_t::sendfile(
	int file_fd, off_t &_offset, size_t &_len, interval_t *timeout
) {
	off_t offset = _offset;
	size_t len = _len;

	while(len) {
		ssize_t res = bq_sendfile(out_fd, file_fd, offset, len, timeout);

		if(res < 0) {
			int err = errno;
			log::level_t _log_level = log_level;

			switch(err) {
				case EINVAL: case EIO: case ENOMEM: _log_level = log::error;
			}

			throw exception_sys_t(_log_level, err, "sendfile: %m");
		}

		if(res == 0) break;

		len -= res;
	}

	_offset = offset;
	_len = len;
}

void bq_conn_fd_t::wait_read(interval_t *timeout) {
	if(!bq_wait_read(in_fd, timeout))
		throw exception_sys_t(log_level, errno, "poll: %m");
}

void bq_conn_fd_t::ctl(int i) {
	if(fd_ctl) (*fd_ctl)(out_fd, i);
}

bq_conn_fd_t::~bq_conn_fd_t() throw() {
	if(dup) ::close(out_fd);
}

} // namespace pd
