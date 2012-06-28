// This file is part of the phantom::io_stream::proto_http module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/http/server.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_stream { namespace proto_http {

class logger_t {
public:
	typedef http::remote_request_t request_t;
	typedef http::local_reply_t reply_t;

	virtual void commit(request_t const &request, reply_t const &rep) const = 0;

protected:
	inline logger_t() throw() { }
	inline ~logger_t() throw() { }

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

}}} // namespace phantom::io_stream::proto_http

#pragma GCC visibility pop
