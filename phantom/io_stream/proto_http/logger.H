// This file is part of the phantom::io_stream::proto_http module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/shared_logger_file.H>

#include <pd/http/server.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_stream { namespace proto_http {

class logger_t : public shared_logger_file_t {
public:
	typedef http::remote_request_t request_t;
	typedef http::local_reply_t reply_t;

	virtual void commit(request_t const &request, reply_t const &rep) = 0;

protected:
	inline logger_t(string_t const &name, config_t const &config) :
		shared_logger_file_t(name, config) { }

	inline ~logger_t() throw() { }
};

}}} // namespace phantom::io_stream::proto_http

#pragma GCC visibility pop
