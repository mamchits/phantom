// This file is part of the phantom program.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "io.H"

#include <pd/bq/bq_job.H>

#include <pd/base/exception.H>

namespace phantom {

void io_t::exec() {
	bq_job_t<typeof(&io_t::run)>::create(
		name, scheduler.bq_thr(), *this, &io_t::run
	);
}

namespace io {
config_binding_sname(io_t);
config_binding_value(io_t, scheduler);
}

} // namespace phantom
