// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "out_fd.H"
#include "exception.H"

#include <unistd.h>
#include <sys/sendfile.h>

namespace pd {

void out_fd_t::flush() {
	iovec outvec[2]; int outcount = 0;

	if(rpos < size) {
		outvec[outcount].iov_base = data + rpos;
		outvec[outcount].iov_len = ((wpos > rpos) ? wpos : size) - rpos;
		++outcount;
	}

	if(wpos > 0 && wpos <= rpos) {
		outvec[outcount].iov_base = data;
		outvec[outcount].iov_len = wpos;
		++outcount;
	}

	if(outcount) {
		ssize_t res = writev(fd, outvec, outcount);
		if(res <= 0) {
			if(res == 0) errno = EREMOTEIO; // Не бывает
			throw exception_sys_t(log_level, errno, "writev: %m");
		}

		if(stat) (*stat) += res;

		rpos += res;
		if(rpos > size) rpos -= size;
	}

	if(rpos == wpos) {
		wpos = 0;
		rpos = size;
	}
}

out_t &out_fd_t::ctl(int i) {
	if(fd_ctl) (*fd_ctl)(fd, i);
	return *this;
}

out_fd_t::~out_fd_t() throw() {
	safe_run(*this, &out_fd_t::flush_all);
}

out_t &out_fd_t::sendfile(int from_fd, off_t &_offset, size_t &_len) {
	flush_all();
	off_t offset = _offset;
	size_t len = _len;

	while(len) {
		ssize_t res = ::sendfile(fd, from_fd, &offset, len);

		if(res < 0) {
			int err = errno;
			log::level_t _log_level = log_level;

			switch(err) {
				case EINVAL: case EIO: case ENOMEM: _log_level = log::error;
			}

			throw exception_sys_t(_log_level, err, "sendfile: %m");
		}

		if(res == 0) break;

		if(stat) (*stat) += res;

		len -= res;
	}

	_offset = offset;
	_len = len;
	return *this;
}

} // namespace pd
