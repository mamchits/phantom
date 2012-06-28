// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/io_benchmark/method.H>

#include <pd/base/fd.H>
#include <pd/base/netaddr.H>
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
}

class method_stream_t : public method_t {
public:
	typedef method_stream::source_t source_t;
	typedef method_stream::transport_t transport_t;
	typedef method_stream::proto_t proto_t;
	typedef method_stream::logger_t logger_t;
	typedef method_stream::conn_t conn_t;
	typedef method_stream::result_t result_t;

private:
	virtual netaddr_t const &dest_addr() const = 0;
	virtual void bind(int fd) const = 0;
	virtual fd_ctl_t const *ctl() const = 0;

	interval_t timeout;
	size_t ibuf_size, obuf_size;
	source_t &source;
	transport_t const &transport;
	proto_t &proto;

	struct loggers_t {
		size_t size;
		logger_t **items;
		inline loggers_t() throw() : size(0), items(NULL) { }
		inline ~loggers_t() throw() { if(items) delete [] items; }

		void commit(
			in_segment_t const &request, in_segment_t &tag, result_t const &res
		) const;

	} loggers;

	size_t network_ind;

	virtual bool test(stat_t &stat) const;
	virtual void init();
	virtual void stat(out_t &out, bool clear, bool hrr_flag) const;
	virtual void fini();
	virtual size_t maxi() const throw();
	virtual descr_t const *descr(size_t ind) const throw();

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
		config::list_t<config::objptr_t<logger_t> > loggers;

		config_t() throw();
		void check(in_t::ptr_t const &p) const;
		inline ~config_t() throw() { }
	};

	method_stream_t(string_t const &, config_t const &);
	~method_stream_t() throw();
};

}} // namespace phantom::io_benchmark

#pragma GCC visibility pop
