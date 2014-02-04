// This file is part of the phantom::io_client::proto_none module.
// Copyright (C) 2010-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/io_client/proto.H>

#include <pd/base/time.H>
#include <pd/base/ref.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_client {

namespace proto_none {
class task_t;
class entry_t;
class instance_t;
}

class proto_none_t : public proto_t {
	typedef proto_none::task_t task_t;
	typedef proto_none::entry_t entry_t;
	typedef proto_none::instance_t instance_t;

	virtual void do_init();
	virtual void do_run() const;
	virtual void do_stat_print() const;
	virtual void do_fini();

	virtual proto_t::instance_t *create_instance(unsigned int rank);

	size_t instances;

public:
	entry_t *entry;
	void put_task(ref_t<task_t> &task) const;

	struct config_t;

	struct prms_t {
		size_t ibuf_size, obuf_size;
		interval_t out_timeout, in_timeout;
		size_t queue_size, quorum;

		prms_t(config_t const &config) throw();
		inline ~prms_t() throw() { }
	};

	prms_t const prms;

	proto_none_t(string_t const &name, config_t const &config) throw();
	~proto_none_t() throw();
};

}} // namespace::io_client

#pragma GCC visibility pop
