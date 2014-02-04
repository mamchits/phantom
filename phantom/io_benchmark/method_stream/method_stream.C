// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "method_stream.H"
#include "logger.H"
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
#include <pd/base/stat_items.H>

namespace phantom { namespace io_benchmark {

MODULE(io_benchmark_method_stream);

namespace method_stream {

class load_t {
	spinlock_t spinlock;
	interval_t real, event;

public:
	inline load_t() throw() :
		spinlock(), real(interval::zero), event(interval::zero) { }

	inline ~load_t() throw() { }

	load_t(load_t const &) = delete;
	load_t &operator=(load_t const &) = delete;

	inline void put(interval_t _real, interval_t _event) {
		spinlock_guard_t guard(spinlock);
		real += _real;
		event += _event;
	}

	typedef load_t val_t;

	class res_t {
	public:
		interval_t real, event;

		inline res_t(load_t &load) throw() {
			spinlock_guard_t guard(load.spinlock);
			real = load.real; load.real = interval::zero;
			event = load.event; load.event = interval::zero;
		}

		inline res_t() throw() :
			real(interval::zero), event(interval::zero) { }

		inline ~res_t() throw() { }

		inline res_t(res_t const &) = default;
		inline res_t &operator=(res_t const &) = default;

		inline res_t &operator+=(res_t const &res) throw() {
			real += res.real;
			event += res.event;

			return *this;
		}
	};

	friend class res_t;
};

typedef stat::count_t conns_t;
typedef stat::count_t icount_t;
typedef stat::count_t ocount_t;
typedef stat::mmcount_t mmtasks_t;

typedef stat::items_t<
	conns_t,
	icount_t,
	ocount_t,
	mmtasks_t,
	load_t
> stat_base_t;

struct stat_t : stat_base_t {
	inline stat_t() throw() : stat_base_t(
		STRING("conns"),
		STRING("in"),
		STRING("out"),
		STRING("mmtasks"),
		STRING("load")
	) { }

	inline ~stat_t() throw() { }

	inline conns_t &conns() throw() { return item<0>(); }
	inline icount_t &icount() throw() { return item<1>(); }
	inline ocount_t &ocount() throw() { return item<2>(); }
	inline mmtasks_t &mmtasks() throw() { return item<3>(); }
	inline load_t &load() throw() { return item<4>(); }
};

struct loggers_t : sarray1_t<logger_t *> {
	inline loggers_t(
		config::list_t<config::objptr_t<logger_t>> const &list
	) :
		sarray1_t<logger_t *>(list) { }

	inline ~loggers_t() throw() { }

	inline void init(string_t const &name) const {
		for(size_t i = 0; i < size; ++i)
			items[i]->init(name);
	}

	inline void run(string_t const &name) const {
		for(size_t i = 0; i < size; ++i)
			items[i]->run(name);
	}

	inline void stat_print(string_t const &name) const {
		stat::ctx_t ctx(CSTR("loggers"), 1);
		for(size_t i = 0; i < size; ++i)
			items[i]->stat_print(name);
	}

	inline void fini(string_t const &name) const {
		for(size_t i = 0; i < size; ++i)
			items[i]->fini(name);
	}

	inline void commit(
		in_segment_t const &request, in_segment_t &tag, result_t const &res
	) const {
		for(size_t i = 0; i < size; ++i) {
			logger_t *logger = items[i];
			if(res.log_level >= logger->level)
				logger->commit(request, tag, res);
		}
	}
};

} // namespace method_stream

method_stream_t::config_t::config_t() throw() :
	ibuf_size(4 * sizeval::kilo), obuf_size(sizeval::kilo),
	timeout(interval::second), source(), transport(), proto(), loggers() { }

void method_stream_t::config_t::check(in_t::ptr_t const &ptr) const {
	if(ibuf_size > sizeval::mega)
		config::error(ptr, "ibuf_size is too big");

	if(ibuf_size < sizeval::kilo)
		config::error(ptr, "ibuf_size is too small");

	if(obuf_size > sizeval::mega)
		config::error(ptr, "obuf_size is too big");

	if(obuf_size < sizeval::kilo)
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

method_stream_t::method_stream_t(string_t const &name, config_t const &config) :
	method_t(name), timeout(config.timeout),
	ibuf_size(config.ibuf_size), obuf_size(config.obuf_size),
	source(*config.source),
	transport(*({
		transport_t const *transport = &method_stream::default_transport;

		if(config.transport)
			transport = config.transport;

		transport;
	})),
	proto(*config.proto), loggers(*new loggers_t(config.loggers)),
	mcount(tags), stat(*new stat_t) { }

method_stream_t::~method_stream_t() throw() {
	delete &stat;
	delete &loggers;
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

namespace method_stream {

#define STREAM_ERROR_LIST \
	STREAM_ERROR(0) \
	STREAM_ERROR(EPERM) \
	STREAM_ERROR(ENOENT) \
	STREAM_ERROR(EIO) \
	STREAM_ERROR(ENXIO) \
	STREAM_ERROR(EBADF) \
	STREAM_ERROR(ENOMEM) \
	STREAM_ERROR(EACCES) \
	STREAM_ERROR(EFAULT) \
	STREAM_ERROR(EBUSY) \
	STREAM_ERROR(EINVAL) \
	STREAM_ERROR(EMFILE) \
	STREAM_ERROR(EPIPE) \
	STREAM_ERROR(ENOSYS) \
	STREAM_ERROR(ENODATA) \
	STREAM_ERROR(ENONET) \
	STREAM_ERROR(EPROTO) \
	STREAM_ERROR(EPROTOTYPE) \
	STREAM_ERROR(ENOPROTOOPT) \
	STREAM_ERROR(EPROTONOSUPPORT) \
	STREAM_ERROR(ESOCKTNOSUPPORT) \
	STREAM_ERROR(EPFNOSUPPORT) \
	STREAM_ERROR(EAFNOSUPPORT) \
	STREAM_ERROR(EADDRINUSE) \
	STREAM_ERROR(EADDRNOTAVAIL) \
	STREAM_ERROR(ENETDOWN) \
	STREAM_ERROR(ENETUNREACH) \
	STREAM_ERROR(ENETRESET) \
	STREAM_ERROR(ECONNABORTED) \
	STREAM_ERROR(ECONNRESET) \
	STREAM_ERROR(ETIMEDOUT) \
	STREAM_ERROR(ECONNREFUSED) \
	STREAM_ERROR(EHOSTDOWN) \
	STREAM_ERROR(EHOSTUNREACH) \
	STREAM_ERROR(EREMOTEIO) \
	STREAM_ERROR(ECANCELED)

static int const vals[] {
#define STREAM_ERROR(x) x,
	STREAM_ERROR_LIST
#undef STREAM_ERROR
};

static size_t const vals_count = sizeof(vals) / sizeof(vals[0]);

static size_t err_idx(int err) {
	size_t idx = 0;
	switch(err) {
#define STREAM_ERROR(x) \
		case x: ++idx;

		STREAM_ERROR_LIST

#undef STREAM_ERROR
	}

	return vals_count - idx;
}

#undef STREAM_ERROR_LIST

struct tags_t : mcount_t::tags_t {
	inline tags_t() : mcount_t::tags_t() { }
	inline ~tags_t() throw() { }

	virtual size_t size() const { return vals_count + 1; }

	virtual void print(out_t &out, size_t idx) const {
		assert(idx <= vals_count);

		if(idx < vals_count) {
			char buf[128];
			char *res = strerror_r(vals[idx], buf, sizeof(buf));
			buf[sizeof(buf) - 1] = '\0';
			out(str_t(res, strlen(res)));
		}
		else
			out(CSTR("Other"));
	}
};

tags_t const tags;

} // namespace method_stream

mcount_t::tags_t const &method_stream_t::tags = method_stream::tags;

namespace method_stream {
bq_spec_decl(conn_t, conn_current);
}

bool method_stream_t::test(times_t &times) const {
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

		++stat.mmtasks();

		_retry:

		result_t res;
		interval_t cur_timeout = timeout;

		try {
			res.proto_code = 0;

			if(!conn) {
				conn = transport.new_connect(connect(cur_timeout), ctl());
				conn->setup_connect();
				++stat.conns();
				res.time_conn = max(res.time_start, timeval::current());
			}
			else
				res.time_conn = res.time_start;

			try {
				char obuf[obuf_size];
				bq_out_t out(*conn, obuf, sizeof(obuf), &stat.ocount());
				out.timeout_set(cur_timeout);

				out.ctl(1);
				out(request).flush_all();
				out.ctl(0);

				res.size_out = request.size();

				cur_timeout = out.timeout_get();
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

			res.time_send = max(res.time_conn, timeval::current());

			conn->wait_read(&cur_timeout);

			res.time_recv = max(res.time_send, timeval::current());

			{
				bq_in_t in(*conn, ibuf_size, &stat.icount());
				in.timeout_set(cur_timeout);
				in_t::ptr_t ptr = in;
				in_t::ptr_t ptr_begin = ptr;

				try {
					if(!proto.reply_parse(ptr, request, res.proto_code, res.log_level)) {
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

					cur_timeout = in.timeout_get();

					res.size_in = ptr - ptr_begin;
					res.reply = in_segment_t(ptr_begin, res.size_in);

					throw;
				}

				if(conn)
					++conn->requests;

				cur_timeout = in.timeout_get();

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
		}
		catch(exception_t const &ex) {
			delete conn;
			conn = NULL;

			res.err = EPROTO;
			res.log_level = logger_t::transport_error;
		}

		timeval_t time_end = timeval::current();

		if(!res.time_conn.is_real())
			res.time_conn = max(res.time_start, time_end);

		if(!res.time_send.is_real())
			res.time_send = max(res.time_conn, time_end);

		if(!res.time_recv.is_real())
			res.time_recv = max(res.time_send, time_end);

		res.time_end = max(res.time_recv, time_end);

		res.interval_event = timeout - cur_timeout;

		--stat.mmtasks();

		interval_t interval_real = res.time_end - res.time_start;

		times.inc(interval_real);

		stat.load().put(interval_real, res.interval_event);
		mcount.inc(method_stream::err_idx(res.err));

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

void method_stream_t::do_init() {
	stat.init();
	mcount.init();
	proto.init(name);
	source.init(name);
	loggers.init(name);
}

void method_stream_t::do_run() const {
	proto.run(name);
	source.run(name);
	loggers.run(name);
}

void method_stream_t::do_stat_print() const {
	stat.print();
	mcount.print();
	proto.stat_print(name);
	source.stat_print(name);
	loggers.stat_print(name);
}

void method_stream_t::do_fini() {
	proto.fini(name);
	source.fini(name);
	loggers.fini(name);
}

}} // namespace phantom::io_benchmark

namespace pd { namespace stat {

typedef phantom::io_benchmark::method_stream::load_t load_t;

template<>
void ctx_t::helper_t<load_t>::print(
	ctx_t &ctx, str_t const &_tag,
	load_t const &, load_t::res_t const &res
) {
	if(ctx.pre(_tag, NULL, 0, NULL, 0)) {
		uint64_t val = res.real > interval::zero ? (res.real - res.event) * 1000 / res.real : 0;

		if(ctx.format == ctx.json) {
			ctx.out.print(val / 1000)('.').print(val % 1000, "03");
		}
		else if(ctx.format == ctx.html) {
			ctx.off();
			ctx.out
				(CSTR("<td>"))
				.print(val / 1000)('.').print(val % 1000, "03")
				(CSTR("</td>"))
			;
		}
	}
}

}} // namespace pd::stat
