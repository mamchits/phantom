// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/in_buf.H>
#include <pd/bq/bq_conn.H>

#pragma GCC visibility push(default)

namespace pd {

class bq_in_t : public in_buf_t {
	virtual size_t readv(iovec *iov, size_t cnt);

	bq_conn_t &conn;

	interval_t timeout_def, timeout_cur;

public:
	inline bq_in_t(
		bq_conn_t &_conn, size_t _page_size, interval_t timeout
	) throw() :
		in_buf_t(_page_size, _conn.log_level), conn(_conn), timeout_def(timeout) {

		timeout_reset();
	}

	inline interval_t timeout_val() const throw() { return timeout_cur; }
	inline void timeout_reset() throw() { timeout_cur = timeout_def; }

	~bq_in_t() throw();
};

} // namespace pd

#pragma GCC visibility pop
