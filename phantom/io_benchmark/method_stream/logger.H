// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/shared_logger_file.H>

#include <pd/base/time.H>
#include <pd/base/config_enum.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_benchmark { namespace method_stream {

struct result_t;

class logger_t : public shared_logger_file_t {
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
	) const = 0;

	struct config_t : shared_logger_file_t::config_t {
		config::enum_t<level_t> level;

		inline config_t() throw() :
			shared_logger_file_t::config_t(), level(all) { }

		inline void check(in_t::ptr_t const &ptr) const {
			shared_logger_file_t::config_t::check(ptr);
		}

		inline ~config_t() throw() { }
	};

protected:
	inline logger_t(string_t const &name, config_t const &config) throw() :
		shared_logger_file_t(name, config), level(config.level) { }

	inline ~logger_t() throw() { }
};

struct result_t {
	timeval_t time_start, time_conn, time_send, time_recv, time_end;
	interval_t interval_event;
	size_t size_in, size_out;
	logger_t::level_t log_level;
	unsigned int err, proto_code;
	in_segment_t reply;

	inline result_t() throw() :
		time_start(timeval::current()), time_conn(timeval::never),
		time_send(timeval::never), time_recv(timeval::never), time_end(timeval::never),
		interval_event(interval::zero),
		size_in(0), size_out(0), log_level(logger_t::all),
		err(0), proto_code(0), reply() { }

	inline ~result_t() throw() { }
};

}}} // namespace phantom::io_benchmark::method_stream

#pragma GCC visibility pop
