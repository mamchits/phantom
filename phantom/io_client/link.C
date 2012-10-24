// This file is part of the phantom::io_client module.
// Copyright (C) 2010-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "link.H"

#include <pd/bq/bq_job.H>
#include <pd/bq/bq_util.H>
#include <pd/bq/bq_conn_fd.H>

#include <pd/base/fd_guard.H>
#include <pd/base/exception.H>
#include <pd/base/config.H>

namespace phantom { namespace io_client {

link_t::link_t(
	link_t *&list, string_t const &_name,
	interval_t _conn_timeout, log::level_t _remote_errors, proto_t &_proto
) :
	list_item_t<link_t>(this, list),
	conn_timeout(_conn_timeout), remote_errors(_remote_errors), name(_name),
	proto_instance(_proto.create_instance(name)) { }

link_t::~link_t() throw() { delete proto_instance; }

void link_t::loop() {
	timeval_t last_conn = timeval_long_ago;

	while(true) {
		try {
			{
				timeval_t now = timeval_current();
				interval_t to_sleep = conn_timeout - (timeval_current() - last_conn);

				if(to_sleep > interval_zero && bq_sleep(&to_sleep) < 0)
					throw exception_sys_t(log::error, errno, "bq_sleep: %m");

			}

			last_conn = timeval_current();

			netaddr_t const &netaddr = remote_netaddr();

			int fd = socket(netaddr.sa->sa_family, SOCK_STREAM, 0);
			if(fd < 0)
				throw exception_sys_t(remote_errors, errno, "socket: %m");

			fd_guard_t fd_guard(fd);

			bq_fd_setup(fd);

			interval_t timeout = conn_timeout;

			if(bq_connect(fd, netaddr.sa, netaddr.sa_len, &timeout) < 0)
				throw exception_sys_t(remote_errors, errno, "connect: %m");

			log_debug("connected");

			bq_conn_fd_t conn(fd, ctl(), remote_errors, /* dup = */ true);

			proto_instance->proc(conn);
		}
		catch(exception_sys_t const &ex) {
			if(ex.errno_val == ECANCELED)
				throw;

			ex.log();
		}
		catch(exception_t const &ex) {
			ex.log();
		}
	}
}

void link_t::do_stat(out_t &, bool) { }

void link_t::do_run() {
	bq_job_t<typeof(&link_t::loop)>::create(
		name, bq_thr_get(), *this, &link_t::loop
	);
}

namespace links {
config_binding_sname(links_t);
config_binding_value(links_t, count);
}

}} // namespace phantom::io_client
