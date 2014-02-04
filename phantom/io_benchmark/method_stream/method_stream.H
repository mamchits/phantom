// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/io_benchmark/method.H>

#include <pd/base/fd.H>
#include <pd/base/netaddr.H>
#include <pd/base/size.H>
#include <pd/base/config.H>
#include <pd/base/config_list.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_benchmark {

class conn_t;

namespace method_stream {
class source_t;
class transport_t;
class proto_t;
class logger_t;
class conn_t;
class result_t;
struct __hidden stat_t;
class __hidden loggers_t;
}

class method_stream_t : public method_t {
public:
	typedef method_stream::source_t source_t;
	typedef method_stream::transport_t transport_t;
	typedef method_stream::proto_t proto_t;
	typedef method_stream::logger_t logger_t;
	typedef method_stream::conn_t conn_t;
	typedef method_stream::result_t result_t;
	typedef method_stream::stat_t stat_t;
	typedef method_stream::loggers_t loggers_t;

private:
	virtual netaddr_t const &dest_addr() const = 0;
	virtual void bind(int fd) const = 0;
	virtual fd_ctl_t const *ctl() const = 0;

	interval_t timeout;
	size_t ibuf_size, obuf_size;
	source_t &source;
	transport_t const &transport;
	proto_t &proto;

	loggers_t &loggers;

	virtual void do_init();
	virtual void do_run() const;
	virtual void do_stat_print() const;
	virtual void do_fini();

	virtual bool test(times_t &times) const;

	static mcount_t::tags_t const &tags;

	mcount_t mutable mcount;

public:
	stat_t &stat;

public:
	int connect(interval_t &timeout) const;

	struct config_t {
		config_binding_type_ref(source_t);
		config_binding_type_ref(transport_t);
		config_binding_type_ref(proto_t);
		config_binding_type_ref(logger_t);

		sizeval_t ibuf_size, obuf_size;
		interval_t timeout;
		config::objptr_t<source_t> source;
		config::objptr_t<transport_t> transport;
		config::objptr_t<proto_t> proto;
		config::list_t<config::objptr_t<logger_t>> loggers;

		config_t() throw();
		void check(in_t::ptr_t const &p) const;
		inline ~config_t() throw() { }
	};

	method_stream_t(string_t const &name, config_t const &config);
	~method_stream_t() throw();
};

}} // namespace phantom::io_benchmark

#pragma GCC visibility pop
