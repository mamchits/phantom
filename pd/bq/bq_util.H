// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/time.H>

#include <sys/socket.h>

#pragma GCC visibility push(default)

namespace pd {

int bq_sleep(interval_t *timeout) throw();

int bq_fd_setup(int fd) throw();

ssize_t bq_read(int fd, void *buf, size_t len, interval_t *timeout);
ssize_t bq_readv(int fd, struct iovec const *vec, int count, interval_t *timeout);

ssize_t bq_write(int fd, void const *buf, size_t len, interval_t *timeout);
ssize_t bq_writev(int fd, struct iovec const *vec, int count, interval_t *timeout);

ssize_t bq_recvfrom(int fd, void *buf, size_t len, struct sockaddr *addr, socklen_t *addrlen, interval_t *timeout);
ssize_t bq_sendto(int fd, const void *buf, size_t len, const struct sockaddr *dest_addr, socklen_t addrlen, interval_t *timeout);

ssize_t bq_sendfile(int fd, int from_fd, off_t &off, size_t size, interval_t *timeout);

int bq_connect(int fd, struct sockaddr const *addr, socklen_t addrlen, interval_t *timeout);
int bq_accept(int fd, struct sockaddr *addr, socklen_t *addrlen, interval_t *timeout, bool force_poll = false);

int bq_poll(int fd, short int &events, interval_t *timeout);

bool bq_wait_read(int fd, interval_t *timeout);
bool bq_wait_write(int fd, interval_t *timeout);

} // namespace pd

#pragma GCC visibility pop
