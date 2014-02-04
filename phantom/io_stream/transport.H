// This file is part of the phantom::io_stream module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/bq/bq_conn.H>

#include <pd/base/fd.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_stream {

class transport_t {
public:
	virtual bq_conn_t *new_connect(
		int fd, fd_ctl_t const *_ctl, log::level_t remote_errors
	) const = 0;

protected:
	inline transport_t() throw() { }
	inline ~transport_t() throw() { }

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

}} // namespace phantom::io_stream

#pragma GCC visibility pop
