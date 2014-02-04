// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "io.H"
#include "scheduler.H"

#include <pd/bq/bq_job.H>

#include <signal.h>
#include <unistd.h>

namespace phantom {

unsigned int io_t::active_count = 0;

template<typename c_t>
struct cleanup_t {
	c_t const &c;
	inline cleanup_t(c_t const &_c) : c(_c) { }
	inline ~cleanup_t() { c(); }
};

void io_t::do_exec() const {
	auto cleanup_f = [this]() -> void {
		if(active && !__sync_sub_and_fetch(&active_count, 1))
			::kill(::getpid(), SIGTERM);
	};

	cleanup_t<typeof(cleanup_f)> cleanup(cleanup_f);

	run();
}

void io_t::exec() const {
	bq_job(&io_t::do_exec)(*this)->run(scheduler.bq_thr());
}

namespace io {
config_binding_sname(io_t);
config_binding_value(io_t, scheduler);
}

} // namespace phantom
