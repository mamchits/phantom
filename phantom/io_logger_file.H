// This file is part of the phantom program.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/io.H>

#include <pd/base/time.H>
#include <pd/base/log_file_handler.H>

#pragma GCC visibility push(default)

namespace phantom {

class io_logger_file_t : public io_t, public log_file_handler_t {
	interval_t check_interval;

	virtual void init();
	virtual void run();
	virtual void stat(out_t &out, bool clear);
	virtual void fini();

public:
	struct config_t : io_t::config_t {
		string_t filename;
		interval_t check_interval;
		string_t header; // handler for dirty hacks.

		inline config_t() throw() :
			io_t::config_t(), filename(), check_interval(interval_second),
			header() { }

		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &ptr) const {
			io_t::config_t::check(ptr);

			if(!filename)
				config::error(ptr, "filename is required");
		}
	};

	inline io_logger_file_t(
		string_t const &name, config_t const &config
	) :
		io_t(name, config),
		log_file_handler_t(config.filename, config.header),
		check_interval(config.check_interval) {
	}

	inline ~io_logger_file_t() throw() { }
};

} // namespace phantom

#pragma GCC visibility pop
