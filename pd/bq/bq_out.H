// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/bq/bq_conn.H>

#include <pd/base/out.H>
#include <pd/base/time.H>
#include <pd/base/log.H>

#pragma GCC visibility push(default)

namespace pd {

class bq_out_t : public out_t {

private:
	bq_conn_t &conn;

	interval_t timeout_def, timeout_cur;

public:
	virtual void flush();
	virtual out_t &ctl(int i);
	virtual out_t &sendfile(int fd, off_t &offset, size_t &size);

	inline bq_out_t(
		bq_conn_t &_conn, char *_data, size_t _size, interval_t timeout
	) :
		out_t(_data, _size), conn(_conn), timeout_def(timeout) {

		timeout_reset();
	}

	inline interval_t timeout_val() const throw() { return timeout_cur; }

	inline void timeout_reset() throw() { timeout_cur = timeout_def; }

	inline void timeout_set(interval_t timeout) { timeout_cur = timeout; }

	~bq_out_t() throw();
};

} // namespace pd

#pragma GCC visibility pop
