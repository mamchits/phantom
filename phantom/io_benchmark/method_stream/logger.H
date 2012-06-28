// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/time.H>
#include <pd/base/config_enum.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_benchmark { namespace method_stream {

struct result_t;

class logger_t {
public:
	enum level_t {
		all = 0,
		proto_warning,
		proto_error,
		transport_error,
		network_error,
	};

	level_t level;

	virtual void commit(
		in_segment_t const &request, in_segment_t const &tag, result_t const &res
	) = 0;

	struct config_t {
		config::enum_t<level_t> level;

		inline config_t() throw() : level(all) { }
		inline void check(in_t::ptr_t const &) const { }
		inline ~config_t() throw() { }
	};

protected:
	inline logger_t(string_t const &, config_t const &config) throw() :
		level(config.level) { }

	inline ~logger_t() throw() { }

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

struct result_t {
	timeval_t time_start, time_conn, time_send, time_recv, time_end;
	interval_t interval_event;
	size_t size_in, size_out;
	logger_t::level_t log_level;
	unsigned int err, proto_code;
	in_segment_t reply;

	inline result_t() throw() :
		time_start(timeval_current()), time_conn(timeval_never),
		time_send(timeval_never), time_recv(timeval_never), time_end(timeval_never),
		interval_event(interval_zero),
		size_in(0), size_out(0), log_level(logger_t::all),
		err(0), proto_code(0), reply() { }

	inline ~result_t() throw() { }
};

}}} // namespace phantom::io_benchmark::method_stream

#pragma GCC visibility pop
