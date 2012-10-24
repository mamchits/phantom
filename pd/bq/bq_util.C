// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_util.H"
#include "bq_thr_impl.I"

#include <pd/base/log.H>

#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/ioctl.h>
#include <errno.h>

namespace pd {

int bq_fd_setup(int fd) throw() {
	int i = 1;
	return ioctl(fd, FIONBIO, &i);
}

bool bq_wait_read(int fd, interval_t *timeout) {
	short int events = POLLIN;
	return bq_success(bq_do_poll(fd, events, timeout, "read_poll"));
}

bool bq_wait_write(int fd, interval_t *timeout) {
	short int events = POLLOUT;
	return bq_success(bq_do_poll(fd, events, timeout, "write_poll"));
}

ssize_t bq_read(int fd, void *buf, size_t len, interval_t *timeout) {
	while(true) {
		ssize_t res = read(fd, buf, len);

		if(res < 0 && errno == EAGAIN) {
			short int events = POLLIN;
			if(bq_success(bq_do_poll(fd, events, timeout, "read")))
				continue;
		}

		return res;
	}
}

ssize_t bq_readv(int fd, struct iovec const *vec, int count, interval_t *timeout) {
	while(true) {
		ssize_t res = readv(fd, vec, count);

		if(res < 0 && errno == EAGAIN) {
			short int events = POLLIN;
			if(bq_success(bq_do_poll(fd, events, timeout, "readv")))
				continue;
		}

		return res;
	}
}

ssize_t bq_write(int fd, void const *buf, size_t len, interval_t *timeout) {
	while(true) {
		ssize_t res = write(fd, buf, len);

		if(res < 0 && errno == EAGAIN) {
			short int events = POLLOUT;
			if(bq_success(bq_do_poll(fd, events, timeout, "write")))
				continue;
		}

		return res;
	}
}

ssize_t bq_writev(int fd, struct iovec const *vec, int count, interval_t *timeout) {
	while(true) {
		ssize_t res = writev(fd, vec, count);

		if(res < 0 && errno == EAGAIN) {
			short int events = POLLOUT;
			if(bq_success(bq_do_poll(fd, events, timeout, "writev")))
				continue;
		}

		return res;
	}
}

ssize_t bq_recvfrom(int fd, void *buf, size_t len, struct sockaddr *addr, socklen_t *addrlen, interval_t *timeout) {
	while(true) {
		ssize_t res = recvfrom(fd, buf, len, 0, addr, addrlen);

		if(res < 0 && errno == EAGAIN) {
			short int events = POLLIN;
			if(bq_success(bq_do_poll(fd, events, timeout, "recvfrom")))
				continue;
		}

		return res;
	}
}

ssize_t bq_sendto(int fd, const void *buf, size_t len, const struct sockaddr *addr, socklen_t addrlen, interval_t *timeout) {
	while(true) {
		ssize_t res = sendto(fd, buf, len, 0, addr, addrlen);

		if(res < 0 && errno == EAGAIN) {
			short int events = POLLOUT;
			if(bq_success(bq_do_poll(fd, events, timeout, "sendto")))
				continue;
		}

		return res;
	}
}

int bq_connect(int fd, struct sockaddr const *addr, socklen_t addrlen, interval_t *timeout) {
	int res = connect(fd, addr, addrlen);

	if(res < 0 && errno == EINPROGRESS) {
		short int events = POLLOUT;
		if(bq_success(bq_do_poll(fd, events, timeout, "connect"))) {
			if(events & POLLERR) {
				int err; socklen_t errlen = sizeof(err);
				getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen);
				errno = err;
				return -1;
			}
			return 0;
		}
		return -1;
	}

	return res;
}

int bq_accept(int fd, struct sockaddr *addr, socklen_t *addrlen, interval_t *timeout, bool force_poll) {
	if(force_poll) {
		short int events = POLLIN;
		if(!bq_success(bq_do_poll(fd, events, timeout, "accept")))
			return -1;
	}

	while(true) {
		int res = accept(fd, addr, addrlen);

		if(res < 0 && errno == EAGAIN) {
			short int events = POLLIN;
			if(bq_success(bq_do_poll(fd, events, timeout, "accept")))
				continue;
		}

		return res;
	}
}

ssize_t bq_sendfile(int fd, int from_fd, off_t &off, size_t size, interval_t *timeout) {
	while(true) {
		ssize_t res = sendfile(fd, from_fd, &off, size);

		if(res < 0 && errno == EAGAIN) {
			short int events = POLLOUT;
			if(bq_success(bq_do_poll(fd, events, timeout, "sendfile")))
				continue;
		}

		return res;
	}
}

int bq_poll(int fd, short int &events, interval_t *timeout) {
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;

	int res = poll(&pfd, 1, 0);

	if(res < 0)
		return res;

	if(res) {
		events = pfd.revents;
		return 0;
	}

	if(!bq_success(bq_do_poll(fd, events, timeout, "poll")))
		return -1;

	return 0;
}

class sleep_item_t : public bq_thr_t::impl_t::item_t {
	virtual void attach() throw();
	virtual void detach() throw();

public:
	inline sleep_item_t(interval_t *timeout) throw() : item_t(timeout) { }

	inline ~sleep_item_t() throw() { }
};

void sleep_item_t::attach() throw() { }

void sleep_item_t::detach() throw() { }

int bq_sleep(interval_t *timeout) throw() {
	sleep_item_t item(timeout);

	bq_err_t err = item.suspend(false, "sleep");

	if(err == bq_timeout) err = bq_ok;

	return bq_success(err) ? 0 : -1;
}

} // namespace pd
