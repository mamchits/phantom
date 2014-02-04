// This file is part of the phantom::io_monitor module.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../io_logger_file.H"
#include "../scheduler.H"
#include "../module.H"
#include "../stat.H"

#include <pd/bq/bq_job.H>
#include <pd/bq/bq_util.H>

#include <pd/base/out_fd.H>
#include <pd/base/config.H>
#include <pd/base/config_list.H>
#include <pd/base/config_enum.H>
#include <pd/base/exception.H>

namespace phantom {

MODULE(io_monitor);

class io_monitor_t : public io_logger_file_t {
	interval_t period;
	bool clear;
	size_t res_no;
	sarray1_t<obj_t const *> list;

	void proc() const;
	void loop() const;
	virtual void run() const;
	virtual void fini();

public:
	struct config_t : io_logger_file_t::config_t {
		config::enum_t<bool> clear;
		interval_t period;
		config::list_t<obj_t const *> list;
		stat_id_t stat_id;

		inline config_t() throw() :
			io_logger_file_t::config_t(),
			clear(false), period(interval::second), list(), stat_id() { }

		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &ptr) const {
			io_logger_file_t::config_t::check(ptr);

			if(period <= interval::zero)
				config::error(ptr, "period must be a positive interval");

			if(!list)
				config::error(ptr, "list is empty");

			if(!stat_id)
				config::error(ptr, "stat_id is required");
		}
	};

	inline io_monitor_t(string_t const &name, config_t const &config) :
		io_logger_file_t(name, config), period(config.period),
		clear(config.clear), res_no(config.stat_id.res_no), list(config.list) { }

	inline ~io_monitor_t() throw() { }
};

namespace io_monitor {
config_binding_sname(io_monitor_t);
config_binding_value(io_monitor_t, clear);
config_binding_value(io_monitor_t, period);
config_binding_value(io_monitor_t, list);
config_binding_value(io_monitor_t, stat_id);
config_binding_parent(io_monitor_t, io_logger_file_t);
config_binding_ctor(io_t, io_monitor_t);
}

void io_monitor_t::proc() const {
	char tag_buf[35];
	size_t tag_len = ({
		out_t out(tag_buf, sizeof(tag_buf));
		out.print(timeval::current(), "+dz.").used();
	});

	guard_t guard(*this);

	char buf[1024];
	out_fd_t out(buf, sizeof(buf), guard.fd, &guard.ocount);

	stat::ctx_t ctx(out, stat::ctx_t::json, res_no, clear, str_t(tag_buf, tag_len), guard.used);

	obj_stat_print(list.items, list.size);

	out.flush_all();
}

void io_monitor_t::loop() const {
	timeval_t now = timeval::current();

	while(true) {
		timeval_t next = now + period;

		interval_t t = next - timeval::current();

		if(t > interval::zero && bq_sleep(&t) < 0)
			throw exception_sys_t(log::error, errno, "bq_sleep: %m");

		now = next;

		proc();
	}
}

void io_monitor_t::run() const {
	bq_job(&io_monitor_t::loop)(*this)->run(scheduler.bq_thr());

	io_logger_file_t::run();
}

void io_monitor_t::fini() { proc(); }

} // namespace phantom
