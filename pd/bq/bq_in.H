// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/bq/bq_conn.H>

#include <pd/base/in_buf.H>
#include <pd/base/stat.H>

#pragma GCC visibility push(default)

namespace pd {

class bq_in_t : public in_buf_t {
	virtual size_t readv(iovec *iov, size_t cnt);

	bq_conn_t &conn;

	interval_t timeout;

	stat::count_t *stat;

public:
	inline bq_in_t(
		bq_conn_t &_conn, size_t _page_size, stat::count_t *_stat = NULL
	) throw() :
		in_buf_t(_page_size, _conn.log_level), conn(_conn),
		timeout(interval::inf), stat(_stat) { }

	inline interval_t timeout_get() const throw() { return timeout; }
	inline void timeout_set(interval_t _timeout) throw() { timeout = _timeout; }

	~bq_in_t() throw();
};

} // namespace pd

#pragma GCC visibility pop
