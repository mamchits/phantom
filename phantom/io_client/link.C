// This file is part of the phantom::io_client module.
// Copyright (C) 2010-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2014, YANDEX LLC.
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

link_t::~link_t() throw() { delete instance; }

void link_t::loop() const {
	timeval_t last_conn = timeval::long_ago;

	while(true) {
		try {
			{
				interval_t to_sleep = conn_timeout - (timeval::current() - last_conn);

				if(to_sleep > interval::zero && bq_sleep(&to_sleep) < 0)
					throw exception_sys_t(log::error, errno, "bq_sleep: %m");
			}

			last_conn = timeval::current();

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

			instance->proc(conn);
		}
		catch(exception_sys_t const &ex) {
			if(ex.errno_val == ECANCELED)
				throw;
		}
		catch(exception_t const &ex) {
		}
	}
}

void link_t::init() { instance->init(); }

void link_t::stat_print() const {
	instance->stat_print();
}

void link_t::run(scheduler_t const &scheduler) const {
	bq_job(&link_t::loop)(*this)->run(scheduler.bq_thr());
}

namespace links {
config_binding_sname(links_t);
config_binding_value(links_t, count);
config_binding_value(links_t, rank);
}

}} // namespace phantom::io_client
