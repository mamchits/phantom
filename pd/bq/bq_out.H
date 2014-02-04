// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/bq/bq_conn.H>

#include <pd/base/out.H>
#include <pd/base/time.H>
#include <pd/base/log.H>
#include <pd/base/stat.H>

#pragma GCC visibility push(default)

namespace pd {

class bq_out_t : public out_t {

private:
	bq_conn_t &conn;

	interval_t timeout;

	stat::count_t *stat;

public:
	virtual void flush();
	virtual out_t &ctl(int i);
	virtual out_t &sendfile(int fd, off_t &offset, size_t &size);

	inline bq_out_t(
		bq_conn_t &_conn, char *_data, size_t _size, stat::count_t *_stat = NULL
	) :
		out_t(_data, _size), conn(_conn), timeout(interval::inf), stat(_stat) { }

	inline interval_t timeout_get() const throw() { return timeout; }
	inline void timeout_set(interval_t _timeout) throw() { timeout = _timeout; }

	~bq_out_t() throw();
};

} // namespace pd

#pragma GCC visibility pop
