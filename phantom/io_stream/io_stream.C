// This file is part of the phantom::io_stream module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "io_stream.H"
#include "transport.H"
#include "proto.H"

#include "../module.H"
#include "../scheduler.H"

#include <pd/bq/bq_job.H>
#include <pd/bq/bq_util.H>
#include <pd/bq/bq_in.H>
#include <pd/bq/bq_out.H>
#include <pd/bq/bq_conn_fd.H>
#include <pd/base/stat.H>
#include <pd/base/stat_items.H>

#include <pd/base/exception.H>
#include <pd/base/fd_guard.H>

namespace phantom {

MODULE(io_stream);

namespace io_stream {

typedef stat::count_t conns_t;
typedef stat::mmcount_t mmconns_t;
typedef stat::count_t reqs_t;
typedef stat::count_t icount_t;
typedef stat::count_t ocount_t;

typedef stat::items_t<
	conns_t,
	mmconns_t,
	reqs_t,
	icount_t,
	ocount_t
> stat_base_t;

struct stat_t : stat_base_t {
	inline stat_t() throw() : stat_base_t(
		STRING("conns"),
		STRING("mmconns"),
		STRING("reqs"),
		STRING("in"),
		STRING("out")
	) { }

	inline ~stat_t() throw() { }

	inline conns_t &conns() throw() { return item<0>(); }
	inline mmconns_t &mmconns() throw() { return item<1>(); }
	inline reqs_t &reqs() throw() { return item<2>(); }
	inline icount_t &icount() throw() { return item<3>(); }
	inline ocount_t &ocount() throw() { return item<4>(); }
};

} // namespae io_stream

io_stream_t::config_t::config_t() throw() :
	io_t::config_t(),
	listen_backlog(20),
	reuse_addr(false), ibuf_size(sizeval::kilo), obuf_size(4 * sizeval::kilo),
	timeout(interval::minute), keepalive(interval::minute),
	force_poll(interval::inf), transport(), proto(),
	multiaccept(false), aux_scheduler(), remote_errors(log::error) { }

void io_stream_t::config_t::check(in_t::ptr_t const &ptr) const {
	io_t::config_t::check(ptr);

	if(listen_backlog > 128 * sizeval::kilo)
		config::error(ptr, "listen_backlog is too big");

	if(ibuf_size > sizeval::mega)
		config::error(ptr, "ibuf_size is too big");

	if(ibuf_size < sizeval::kilo)
		config::error(ptr, "ibuf_size is too small");

	if(obuf_size > sizeval::mega)
		config::error(ptr, "obuf_size is too big");

	if(obuf_size < sizeval::kilo)
		config::error(ptr, "obuf_size is too small");

	if(timeout > interval::hour)
		config::error(ptr, "timeout is too big");

	if(!proto)
		config::error(ptr, "proto is required");
};

namespace io_stream {
config_binding_sname(io_stream_t);
config_binding_type(io_stream_t, acl_t);
config_binding_type(io_stream_t, transport_t);
config_binding_type(io_stream_t, proto_t);
config_binding_value(io_stream_t, listen_backlog);
config_binding_value(io_stream_t, reuse_addr);
config_binding_value(io_stream_t, ibuf_size);
config_binding_value(io_stream_t, obuf_size);
config_binding_value(io_stream_t, timeout);
config_binding_value(io_stream_t, keepalive);
config_binding_value(io_stream_t, force_poll);
config_binding_value(io_stream_t, transport);
config_binding_value(io_stream_t, proto);
config_binding_value(io_stream_t, multiaccept);
config_binding_value(io_stream_t, aux_scheduler);
config_binding_value(io_stream_t, remote_errors);
config_binding_parent(io_stream_t, io_t);
}

namespace io_stream {

class transport_default_t : public transport_t {
	virtual bq_conn_t *new_connect(
		int fd, fd_ctl_t const *_ctl, log::level_t remote_errors
	) const;

public:
	inline transport_default_t() throw() { }
	inline ~transport_default_t() throw() { }
};

bq_conn_t *transport_default_t::new_connect(
	int fd, fd_ctl_t const *_ctl, log::level_t remote_errors
) const {
	return new bq_conn_fd_t(fd, _ctl, remote_errors);
}

static transport_default_t const default_transport;

} // namespace io_stream

io_stream_t::io_stream_t(string_t const &name, config_t const &config) :
	io_t(name, config),
	fd(-1),
	listen_backlog(config.listen_backlog),
	reuse_addr(config.reuse_addr),
	ibuf_size(config.ibuf_size), obuf_size(config.obuf_size),
	timeout(config.timeout),
	keepalive(config.keepalive), force_poll(config.force_poll),
	transport(*({
		transport_t const *transport = &io_stream::default_transport;

		if(config.transport)
			transport = config.transport;

		transport;
	})),
	proto(*config.proto), multiaccept(config.multiaccept),
	aux_scheduler(config.aux_scheduler), remote_errors(config.remote_errors),
	stat(*new stat_t) { }

io_stream_t::~io_stream_t() throw() { delete &stat; }

void io_stream_t::init() {
	netaddr_t const &netaddr = bind_addr();

	fd = socket(netaddr.sa->sa_family, SOCK_STREAM, 0);
	if(fd < 0)
		throw exception_sys_t(log::error, errno, "socket: %m");

	try {
		bq_fd_setup(fd);
		fd_setup(fd);

		if(reuse_addr) {
			int i = 1;
			if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) < 0)
				throw exception_sys_t(log::error, errno, "setsockopt, SO_REUSEADDR, 1: %m");
		}

		if(::bind(fd, netaddr.sa, netaddr.sa_len) < 0)
			throw exception_sys_t(log::error, errno, "bind: %m");

		if(::listen(fd, listen_backlog) < 0)
			throw exception_sys_t(log::error, errno, "listen: %m");

	}
	catch(...) {
		::close(fd);
		fd = -1;
		throw;
	}

	stat.init();
	proto.init(name);
}

void io_stream_t::conn_proc(int fd, netaddr_t *netaddr) const {
	fd_guard_t fd_guard(fd);

	class netaddr_guard_t {
		netaddr_t *netaddr;
	public:
		inline netaddr_guard_t(netaddr_t *_netaddr) throw() : netaddr(_netaddr) { }
		inline ~netaddr_guard_t() throw() { delete netaddr; }
	} netaddr_guard(netaddr);

	bq_fd_setup(fd);

	bq_conn_t *conn = transport.new_connect(fd, ctl(), remote_errors);

	class conn_guard_t {
		bq_conn_t *conn;
		io_stream::mmconns_t &mmconns;
	public:
		inline conn_guard_t(bq_conn_t *_conn, stat_t &stat) throw() :
			conn(_conn), mmconns(stat.mmconns()) {

			++mmconns;
		}

		inline ~conn_guard_t() throw() {
			--mmconns;

			delete conn;
		}
	} conn_guard(conn, stat);

	conn->setup_accept();

	netaddr_t const &local_addr = bind_addr();

	bq_in_t in(*conn, ibuf_size, &stat.icount());

	for(bool work = true; work;) {
		{
			char obuf[obuf_size];
			bq_out_t out(*conn, obuf, sizeof(obuf), &stat.ocount());

			in_t::ptr_t ptr(in);

			do {
				in.timeout_set(timeout);
				out.timeout_set(timeout);

				work = proto.request_proc(ptr, out, local_addr, *netaddr);
				++stat.reqs();

				if(!work)
					break;

			} while(in.truncate(ptr));
		}

		if(work) {
			short int events = POLLIN;
			interval_t _keepalive = keepalive;
			if(bq_poll(fd, events, &_keepalive) < 0)
				break;
		}
	}

	conn->shutdown();
}

void io_stream_t::loop(int afd, bool conswitch) const {
	fd_guard_t fd_guard(afd);
	timeval_t last_poll = timeval::current();

	while(true) {
		bool poll_before = false;

		if(force_poll.is_real()) {
			timeval_t now = timeval::current();
			if(now - last_poll > force_poll) {
				poll_before = true;
				last_poll = now;
			}
		}

		netaddr_t *netaddr = new_netaddr();

		int nfd = bq_accept(afd, netaddr->sa, &netaddr->sa_len, NULL, poll_before);

		if(nfd < 0) {
			delete netaddr;
			throw exception_sys_t(log::error, errno, "accept: %m");
		}

		try {
			string_t _name = string_t::ctor_t(netaddr->print_len()).print(*netaddr);

			++stat.conns();

			bq_thr_t *thr = aux_scheduler
				? aux_scheduler->bq_thr()
				: (conswitch ? scheduler.bq_thr() : bq_thr_get())
			;
			log::handler_t handler(_name);
			bq_job(&io_stream_t::conn_proc)(*this, nfd, netaddr)->run(thr);
		}
		catch(exception_t const &ex) {
			::close(nfd);
			delete netaddr;
		}
		catch(...) {
			::close(nfd);
			delete netaddr;
			throw;
		}
	}
}

void io_stream_t::run() const {
	proto.run(name);

	if(multiaccept) {
		size_t bq_n = scheduler.bq_n();

		char const *fmt = log::number_fmt(bq_n);

		for(size_t i = 0; i < bq_n; ++i) {
			try {
				string_t _name = string_t::ctor_t(5).print(i, fmt);
				log::handler_t handler(_name);

				int _fd = ::dup(fd);
				if(_fd < 0)
					throw exception_sys_t(log::error, errno, "dup: %m");

				fd_guard_t _fd_guard(_fd);

				bq_job(&io_stream_t::loop)(*this, _fd, false)->run(scheduler.bq_thr(i));

				_fd_guard.relax();
			}
			catch(exception_t const &) { }
		}
	}
	else {
		int _fd = ::dup(fd);
		if(_fd < 0)
			throw exception_sys_t(log::error, errno, "dup: %m");

		loop(_fd, true);
	}
}

void io_stream_t::stat_print() const {
	stat.print();
	proto.stat_print(name);
}

void io_stream_t::fini() {
	::close(fd);
}

} // namespace phantom
