// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/bq/bq_conn.H>

#include <pd/base/fd.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_benchmark { namespace method_stream {

class conn_t {
protected:
	int fd;

public:
	size_t requests;
	virtual operator bq_conn_t &() = 0;

	inline bq_conn_t &bq_conn() { return *this; }

	inline void setup_connect() { bq_conn().setup_connect(); }
	inline void shutdown() { bq_conn().shutdown(); }

	inline void wait_read(interval_t *timeout) { bq_conn().wait_read(timeout); }

	inline conn_t(int _fd) : fd(_fd), requests(0) { }
	virtual ~conn_t() throw();
};

class transport_t {
public:
	virtual conn_t *new_connect(int fd, fd_ctl_t const *_ctl) const = 0;

protected:
	inline transport_t() throw() { }
	inline ~transport_t() throw() { }

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

}}} // namespace phantom::io_benchmark::method_stream
