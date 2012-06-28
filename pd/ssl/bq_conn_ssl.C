// This file is part of the pd::ssl library.
// Copyright (C) 2011, 2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011, 2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_conn_ssl.H"

#include "log.I"

#include <pd/bq/bq_util.H>

#include <openssl/ssl.h>

#include <sys/mman.h>

namespace pd {

bq_conn_ssl_t::bq_conn_ssl_t(
	int _fd, ssl_ctx_t const &ctx, interval_t _timeout,
	fd_ctl_t const *_fd_ctl, log::level_t _log_level
) :
	bq_conn_t(_log_level), internal(NULL), timeout(_timeout),
	fd(_fd), fd_ctl(_fd_ctl) {

	SSL *ssl = SSL_new((SSL_CTX *)ctx.internal);

	if(!ssl) {
		log_openssl_error(log::error);
		throw exception_log_t(log::error, "SSL_new");
	}

	if(SSL_set_fd(ssl, fd) == 0) {
		SSL_free(ssl);
		log_openssl_error(log::error);
		throw exception_log_t(log::error, "SSL_set_fd");
	}

	assert(fd == SSL_get_fd(ssl));

	internal = ssl;
}

static inline int socket_err(int fd) {
	int err; socklen_t errlen = sizeof(err);
	getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen);
	errno = err;
	return err;
}

static int ssl_wait_loop(SSL *ssl, int res, int &SSL_res, interval_t *timeout) throw() {
	int result = SSL_get_error(ssl, res);

	switch(result) {
		case SSL_ERROR_WANT_WRITE:
			//log_debug("SSL_ERROR_WANT_WRITE");
			if(bq_wait_write(SSL_get_fd(ssl), timeout))
				return 0;
			break;

		case SSL_ERROR_WANT_READ:
			//log_debug("SSL_ERROR_WANT_READ");
			if(bq_wait_read(SSL_get_fd(ssl), timeout))
				return 0;
			break;
	}

	SSL_res = result;

	return -1;
}

static int do_read(
	SSL *ssl, char *data, size_t len, int &SSL_res, interval_t *timeout
) throw() {
	while(len) {
		int res = SSL_read(ssl, data, len);
		if(res >= 0) return res;
		else
			if(ssl_wait_loop(ssl, res, SSL_res, timeout) < 0)
				return -1;
	}
	return 0;
}

static int do_write(
	SSL *ssl, char const *data, size_t len, int &SSL_res, interval_t *timeout
) throw() {
	while(len) {
		int res = SSL_write(ssl, data, len);
		if(res >= 0) return res;
		else
			if(ssl_wait_loop(ssl, res, SSL_res, timeout) < 0)
				return -1;
	}
	return 0;
}

static bool do_accept(SSL *ssl, int &SSL_res, interval_t *timeout) throw() {
	SSL_set_accept_state(ssl);

	while(true) {
		int res = SSL_do_handshake(ssl);
		if(res > 0) return true;
		else
			if(ssl_wait_loop(ssl, res, SSL_res, timeout) < 0)
				break;
	}
	return false;
}

static bool do_connect(SSL *ssl, int &SSL_res, interval_t *timeout) throw() {
	SSL_set_connect_state(ssl);

	while(true) {
		int res = SSL_connect(ssl);
		if(res > 0) return true;
		else
			if(ssl_wait_loop(ssl, res, SSL_res, timeout) < 0)
				break;
	}
	return false;
}

static bool do_shutdown(SSL *ssl, int &SSL_res, interval_t *timeout) throw() {
	while(true) {
		int res = SSL_shutdown(ssl);

		if(res > 0) return true;
		if(res == 0) continue;          // do bidirectional shutdown

		if(ssl_wait_loop(ssl, res, SSL_res, timeout) < 0) {
			if(
				SSL_res == SSL_ERROR_SYSCALL && !socket_err(SSL_get_fd(ssl))
			)
				return true;

			break;
		}
	}
	return false;
}

void bq_conn_ssl_t::setup_accept() {
	interval_t cur_timeout = timeout;
	int SSL_res = 0;
	if(!do_accept((SSL *)internal, SSL_res, &cur_timeout)) {
		int err = errno ?: socket_err(fd);

		log_openssl_error(log_level);

		if(err)
			throw exception_sys_t(log_level, err, "SSL_accept: %m");
		else
			throw exception_sys_t(log_level, EPROTO, "SSL_accept error %d", SSL_res);
	}
}

void bq_conn_ssl_t::setup_connect() {
	interval_t cur_timeout = timeout;
	int SSL_res = 0;
	if(!do_connect((SSL *)internal, SSL_res, &cur_timeout)) {
		int err = errno ?: socket_err(fd);

		log_openssl_error(log_level);

		if(err)
			throw exception_sys_t(log_level, err, "SSL_connect: %m");
		else
			throw exception_sys_t(log_level, EPROTO, "SSL_connect error %d", SSL_res);
	}
}

void bq_conn_ssl_t::shutdown() {
	interval_t cur_timeout = timeout;
	int SSL_res = 0;
	if(!do_shutdown((SSL *)internal, SSL_res, &cur_timeout)) {
		int err = errno ?: socket_err(fd);

		log_openssl_error(log_level);

		if(err)
			throw exception_sys_t(log_level, err, "SSL_shutdown: %m");
		else
			throw exception_sys_t(log_level, EPROTO, "SSL_shutdown error %d", SSL_res);
	}

	::shutdown(fd, 2);
}


size_t bq_conn_ssl_t::readv(iovec *iov, size_t cnt, interval_t *timeout) {
	int SSL_res = 0;
	if(cnt == 1) {
		ssize_t res = do_read((SSL *)internal, (char *)iov->iov_base, iov->iov_len, SSL_res, timeout);
		if(res < 0) {
			int err = errno ?: socket_err(fd);

			log_openssl_error(log_level);

			if(err)
				throw exception_sys_t(log_level, err, "SSL_read: %m");
			else
				throw exception_sys_t(log_level, EPROTO, "SSL_read error %d", SSL_res);
		}

		return res;
	}

	size_t len = 0;

	for(size_t i = 0; i < cnt; i++)
		len += iov[i].iov_len;

	if(!len) return 0;
	char buf[len];

	ssize_t res = do_read((SSL *)internal, buf, len, SSL_res, timeout);

	if(res < 0) {
		int err = errno ?: socket_err(fd);

		log_openssl_error(log_level);

		if(err)
			throw exception_sys_t(log_level, err, "SSL_read: %m");
		else
			throw exception_sys_t(log_level, EPROTO, "SSL_read error %d", SSL_res);
	}

	for(size_t i = 0, readed = res, writen = 0; writen < readed; i++) {
		size_t write_chunk = min(iov[i].iov_len, readed - writen);
		memcpy(iov[i].iov_base, &buf[writen], write_chunk);
		writen += write_chunk;
	}

	return res;
}

size_t bq_conn_ssl_t::writev(iovec const *iov, size_t cnt, interval_t *timeout) {
	int SSL_res = 0;
	if(cnt == 1) {
		ssize_t res = do_write((SSL *)internal, (char const *)iov->iov_base, iov->iov_len, SSL_res, timeout);
		if(res <= 0) {
			int err = errno ?: socket_err(fd);

			log_openssl_error(log_level);

			if(err)
				throw exception_sys_t(log_level, err, "SSL_write: %m");
			else
				throw exception_sys_t(log_level, EPROTO, "SSL_write error %d", SSL_res);
		}

		return res;
	}

	size_t len = 0;
	for(size_t i = 0; i < cnt; i++)
		len += iov[i].iov_len;

	if(!len) return 0;

	char buf[len];
	char *bufp = buf;

	for(size_t i = 0; i < cnt; i++) {
		memcpy(bufp, iov[i].iov_base, iov[i].iov_len);

		bufp += iov[i].iov_len;
	}

	ssize_t res = do_write((SSL *)internal, buf, len, SSL_res, timeout);

	if(res <= 0) {
		int err = errno ?: socket_err(fd);

		log_openssl_error(log_level);

		if(err)
			throw exception_sys_t(log_level, err, "SSL_write: %m");
		else
			throw exception_sys_t(log_level, EPROTO, "SSL_write error %d", SSL_res);
	}

	return res;
}

void bq_conn_ssl_t::sendfile(
	int file_fd, off_t &_offset, size_t &_len, interval_t *timeout
) {
	int SSL_res = 0;
	size_t len = _len;

	char const *mem = (char const *)mmap(NULL, len, PROT_READ, MAP_SHARED, file_fd, _offset);

	if(mem == (char const *)MAP_FAILED)
		throw exception_sys_t(log::error, errno, "mmap: %m");

	char const *mptr = mem;

	while(len) {
		ssize_t res = do_write((SSL *)internal, mptr, len, SSL_res, timeout);

		if(res < 0) {
			int err = errno;

			log_openssl_error(log_level);

			munmap((void *)mem, _len);

			if(err)
				throw exception_sys_t(log_level, err, "SSL_write: %m");
			else
				throw exception_sys_t(log_level, EPROTO, "SSL_write error %d", SSL_res);
		}

		len -= res;
		mptr += res;
	}

	munmap((void *)mem, _len);

	_offset += (_len - len);
	_len = len;
}

void bq_conn_ssl_t::wait_read(interval_t *timeout) {
	if(bq_wait_read(fd, timeout) < 0)
		throw exception_sys_t(log_level, errno, "poll: %m");
}

void bq_conn_ssl_t::ctl(int i) {
	if(fd_ctl) (*fd_ctl)(fd, i);
}

bq_conn_ssl_t::~bq_conn_ssl_t() throw() {
	SSL_free((SSL *)internal);
}

} // namespace pd
