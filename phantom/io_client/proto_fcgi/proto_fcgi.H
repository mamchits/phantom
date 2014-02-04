// This file is part of the phantom::io_client::proto_fcgi module.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/io_client/proto.H>

#include <pd/http/server.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_client {

namespace proto_fcgi {
class entry_t;
class instance_t;
}

class proto_fcgi_t : public proto_t {
	typedef proto_fcgi::entry_t entry_t;
	typedef proto_fcgi::instance_t instance_t;

	typedef http::remote_request_t request_t;
	typedef http::local_reply_t reply_t;

	virtual void do_init();
	virtual void do_run() const;
	virtual void do_stat_print() const;
	virtual void do_fini();

	virtual proto_t::instance_t *create_instance(unsigned int _rank);

	size_t instances;

public:
	entry_t *entry;

	bool proc(
		request_t const &request, reply_t &reply,
		interval_t *timeout, string_t const &root
	) const;

	struct config_t;

	struct prms_t {
		size_t ibuf_size, obuf_size;
		size_t queue_size, quorum;
		interval_t in_timeout, out_timeout;

		prms_t(config_t const &config) throw();
		inline ~prms_t() throw() { }
	};
	prms_t const prms;

	proto_fcgi_t(string_t const &name, config_t const &config) throw();
	~proto_fcgi_t() throw();

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

}} // namespace::io_client

#pragma GCC visibility pop
