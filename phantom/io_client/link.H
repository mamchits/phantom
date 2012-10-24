// This file is part of the phantom::io_client module.
// Copyright (C) 2010-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/io_client/proto.H>

#include <pd/base/netaddr.H>
#include <pd/base/list.H>
#include <pd/base/size.H>
#include <pd/base/time.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_client {

class link_t : public list_item_t<link_t> {
	interval_t const conn_timeout;
	log::level_t remote_errors;
	string_t name;
	proto_instance_t *proto_instance;

	virtual netaddr_t const &remote_netaddr() const throw() = 0;
	virtual fd_ctl_t const *ctl() const throw() = 0;

	void loop();
	void do_run();
	void do_stat(out_t &out, bool clear);

public:
	inline void run() {
		if(this) {
			next->run();
			do_run();
		}
	}

	inline void stat(out_t &out, bool clear) {
		if(this) {
			next->stat(out, clear);
			do_stat(out, clear);
		}
	}

protected:
	link_t(
		link_t *&list, string_t const &_name,
		interval_t _conn_timeout, log::level_t _remote_errors, proto_t &_proto
	);

public:
	virtual ~link_t() throw();
};

class links_t {
protected:
	string_t name;
	size_t count;

public:
	struct config_t {
		sizeval_t count;

		inline config_t() : count(1) { }
		inline void check(in_t::ptr_t const &) const { }
		inline ~config_t() throw() { }
	};

	virtual void create(
		link_t *&list, string_t const &client_name,
		interval_t _conn_timeout, log::level_t _remote_errors, proto_t &_proto
	) const = 0;

protected:
	inline links_t(string_t const &_name, config_t const &config) :
		name(_name), count(config.count) { }

	inline ~links_t() throw() { }

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

}} // namespace phantom::io_client

#pragma GCC visibility pop
