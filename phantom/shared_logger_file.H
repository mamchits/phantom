// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/shared.H>
#include <phantom/scheduler.H>

#include <pd/base/time.H>
#include <pd/base/log_file.H>
#include <pd/base/config.H>

#pragma GCC visibility push(default)

namespace phantom {

class shared_logger_file_t : public shared_t, public log_file_t {
	interval_t check_interval;
	scheduler_t const &scheduler;

protected:
	virtual void do_init();
	virtual void do_run() const;
	virtual void do_stat_print() const;
	virtual void do_fini();

	void loop() const;

public:
	struct config_t {
		string_t filename;
		interval_t check_interval;
		config::objptr_t<scheduler_t> scheduler;
		string_t header; // handler for dirty hacks.

		inline config_t() throw() :
			filename(), check_interval(interval::second), scheduler(),
			header() { }

		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &ptr) const {
			if(!scheduler)
				config::error(ptr, "scheduler is required");

			if(!filename)
				config::error(ptr, "filename is required");
		}
	};

	inline shared_logger_file_t(
		string_t const &name, config_t const &config
	) :
		shared_t(name),
		log_file_t(config.filename, config.header),
		check_interval(config.check_interval), scheduler(*config.scheduler) { }

	inline ~shared_logger_file_t() throw() { }
};

} // namespace phantom

#pragma GCC visibility pop
