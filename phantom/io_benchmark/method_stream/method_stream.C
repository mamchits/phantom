// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "method_stream.H"
#include "transport.H"
#include "proto.H"
#include "source.H"

#include "../../module.H"

#include <pd/bq/bq_in.H>
#include <pd/bq/bq_out.H>
#include <pd/bq/bq_conn_fd.H>
#include <pd/bq/bq_util.H>
#include <pd/bq/bq_spec.H>

#include <pd/base/exception.H>
#include <pd/base/fd_guard.H>

namespace phantom { namespace io_benchmark {

MODULE(io_benchmark_method_stream);

method_stream_t::config_t::config_t() throw() :
	ibuf_size(4 * sizeval_kilo), obuf_size(sizeval_kilo),
	timeout(interval_second), source(), transport(), proto(), loggers() { }

void method_stream_t::config_t::check(in_t::ptr_t const &ptr) const {
	if(ibuf_size > sizeval_mega)
		config::error(ptr, "ibuf_size is too big");

	if(ibuf_size < sizeval_kilo)
		config::error(ptr, "ibuf_size is too small");

	if(obuf_size > sizeval_mega)
		config::error(ptr, "obuf_size is too big");

	if(obuf_size < sizeval_kilo)
		config::error(ptr, "obuf_size is too small");

	if(!source)
		config::error(ptr, "source is required");

	if(!proto)
		config::error(ptr, "proto is required");
}

namespace method_stream {
config_binding_sname(method_stream_t);
config_binding_value(method_stream_t, timeout);
config_binding_value(method_stream_t, ibuf_size);
config_binding_value(method_stream_t, obuf_size);
config_binding_type(method_stream_t, source_t);
config_binding_value(method_stream_t, source);
config_binding_type(method_stream_t, transport_t);
config_binding_value(method_stream_t, transport);
config_binding_type(method_stream_t, proto_t);
config_binding_value(method_stream_t, proto);
config_binding_type(method_stream_t, logger_t);
config_binding_value(method_stream_t, loggers);
}

namespace method_stream {

class transport_default_t : public transport_t {
	class conn_default_t : public conn_t {
		bq_conn_fd_t bq_conn;

		virtual operator bq_conn_t &();

	public:
		inline conn_default_t(int fd, fd_ctl_t const *_ctl) :
			conn_t(fd), bq_conn(fd, _ctl) { }

		virtual ~conn_default_t() throw();
	};

	virtual conn_t *new_connect(int fd, fd_ctl_t const *_ctl) const;

public:
	inline transport_default_t() throw() : transport_t() { }
	inline ~transport_default_t() throw() { }
};

transport_default_t::conn_default_t::~conn_default_t() throw() { }

conn_t *transport_default_t::new_connect( int fd, fd_ctl_t const *_ctl) const {
	return new conn_default_t(fd, _ctl);
}

transport_default_t::conn_default_t::operator bq_conn_t &() {
	return bq_conn;
}

static transport_default_t const default_transport;

} // namespace method_stream

method_stream_t::method_stream_t(string_t const &, config_t const &config) :
	method_t(), timeout(config.timeout),
	ibuf_size(config.ibuf_size), obuf_size(config.obuf_size),
	source(*config.source),
	transport(*({
		transport_t const *transport = &method_stream::default_transport;

		if(config.transport)
			transport = config.transport;

		transport;
	})),
	proto(*config.proto), loggers() {

	for(typeof(config.loggers.ptr()) lptr = config.loggers; lptr; ++lptr)
		++loggers.size;

	loggers.items = new logger_t *[loggers.size];

	size_t i = 0;
	for(typeof(config.loggers.ptr()) lptr = config.loggers; lptr; ++lptr)
		loggers.items[i++] = lptr.val();

	network_ind = proto.maxi();
}

int method_stream_t::connect(interval_t &timeout) const {
	netaddr_t const &netaddr = dest_addr();

	int fd = socket(netaddr.sa->sa_family, SOCK_STREAM, 0);
	if(fd < 0)
		throw exception_sys_t(log::error, errno, "socket: %m");

	try {
		bq_fd_setup(fd);

		bind(fd);

		if(bq_connect(fd, netaddr.sa, netaddr.sa_len, &timeout) < 0)
			throw exception_sys_t(log::error, errno, "connect: %m");
	}
	catch(...) {
		::close(fd);
		throw;
	}

	return fd;
}

method_stream_t::~method_stream_t() throw() { }

void method_stream_t::loggers_t::commit(
	in_segment_t const &request, in_segment_t &tag, result_t const &res
) const {
	for(size_t i = 0; i < size; ++i) {
		logger_t *logger = items[i];
		if(res.log_level >= logger->level)
			logger->commit(request, tag, res);
	}
}

namespace method_stream {
bq_spec_decl(conn_t, conn_current);
}

bool method_stream_t::test(stat_t &stat) const {
	conn_t *&conn = method_stream::conn_current;

	try {
		in_segment_t request;
		in_segment_t tag;

		if(!source.get_request(request, tag)) {
			if(conn) {
				conn->shutdown();
				delete conn;
				conn = NULL;
			}
			return false;
		}

		stat_t::tcount_guard_t tcount_guard(stat);

		_retry:

		result_t res;
		bool event = false;
		interval_t cur_timeout = timeout;

		try {
			res.proto_code = 0;

			if(!conn) {
				conn = transport.new_connect(connect(cur_timeout), ctl());
				conn->setup_connect();
				event = true;
				res.time_conn = max(res.time_start, timeval_current());
			}
			else
				res.time_conn = res.time_start;

			try {
				char obuf[obuf_size];
				bq_out_t out(*conn, obuf, sizeof(obuf), cur_timeout);

				out.ctl(1);
				out(request).flush_all();
				out.ctl(0);

				res.size_out = request.size();

				cur_timeout = out.timeout_val();
			}
			catch(exception_sys_t const &ex) {
				if(conn->requests && ex.errno_val == EPIPE) {
					log_warning("Restart connecton #1");
					delete conn;
					conn = NULL;
					goto _retry;
				}
				throw;
			}

			res.time_send = max(res.time_conn, timeval_current());

			conn->wait_read(&cur_timeout);

			res.time_recv = max(res.time_send, timeval_current());

			{
				bq_in_t in(*conn, ibuf_size, cur_timeout);
				in_t::ptr_t ptr = in;
				in_t::ptr_t ptr_begin = ptr;

				try {
					if(!proto.reply_parse(ptr, request, res.proto_code, stat, res.log_level)) {
						conn->shutdown();
						delete conn;
						conn = NULL;
					}
				}
				catch(exception_sys_t const &ex) {
					ptr.seek_end();

					if(conn->requests && ptr == ptr_begin && ex.errno_val != ECANCELED) {
						log_warning("Restart connecton #2");
						delete conn;
						conn = NULL;
						goto _retry;
					}

					cur_timeout = in.timeout_val();

					res.size_in = ptr - ptr_begin;
					res.reply = in_segment_t(ptr_begin, res.size_in);

					throw;
				}

				if(conn)
					++conn->requests;

				cur_timeout = in.timeout_val();

				res.size_in = ptr - ptr_begin;
				res.reply = in_segment_t(ptr_begin, res.size_in);
			}
		}
		catch(exception_sys_t const &ex) {
			res.err = ex.errno_val;

			delete conn;
			conn = NULL;

			if(res.err == ECANCELED)
				throw;

			if(res.err == EPROTO)
				res.log_level = logger_t::transport_error;
			else
				res.log_level = logger_t::network_error;

			str_t msg = ex.msg();
			log_warning("Connection closed: %.*s", (int)msg.size(), msg.ptr());
		}
		catch(exception_t const &ex) {
			delete conn;
			conn = NULL;

			res.err = EPROTO;
			res.log_level = logger_t::transport_error;

			str_t msg = ex.msg();
			log_warning("Connection closed: %.*s", (int)msg.size(), msg.ptr());
		}

		timeval_t time_end = timeval_current();

		if(!res.time_conn.is_real())
			res.time_conn = max(res.time_start, time_end);

		if(!res.time_send.is_real())
			res.time_send = max(res.time_conn, time_end);

		if(!res.time_recv.is_real())
			res.time_recv = max(res.time_send, time_end);

		res.time_end = max(res.time_recv, time_end);

		res.interval_event = timeout - cur_timeout;

		{
			thr::spinlock_guard_t guard(stat.spinlock);

			stat.update_time(res.time_end - res.time_start, res.interval_event);

			if(event) stat.event();

			stat.update(network_ind, res.err);
			if(!res.err) {
				stat.update_size(res.size_in, res.size_out);
			}
		}

		loggers.commit(request, tag, res);
	}
	catch(...) {
		if(conn) {
			delete conn;
			conn = NULL;
		}
		throw;
	}

	return true;
}

void method_stream_t::init() {
	source.init();
}

void method_stream_t::stat(out_t &out, bool clear, bool hrr_flag) const {
	source.stat(out, clear, hrr_flag);
}

void method_stream_t::fini() {
	source.fini();
}

class network_descr_t : public descr_t {
	static size_t const max_errno = 140;

	virtual size_t value_max() const { return max_errno; }

	virtual void print_header(out_t &out) const {
		out(CSTR("network"));
	}

	virtual void print_value(out_t &out, size_t value) const {
		if(value < max_errno) {
			char buf[128];
			char *res = strerror_r(value, buf, sizeof(buf));
			buf[sizeof(buf) - 1] = '\0';
			out(str_t(res, strlen(res)));
		}
		else
			out(CSTR("Unknown error"));
	}
public:
	inline network_descr_t() throw() : descr_t() { }
	inline ~network_descr_t() throw() { }
};

static network_descr_t const network_descr;

size_t method_stream_t::maxi() const throw() {
	return network_ind + 1;
}

descr_t const *method_stream_t::descr(size_t ind) const throw() {
	return (ind == network_ind) ? &network_descr : proto.descr(ind);
}

}} // namespace phantom::io_benchmark
