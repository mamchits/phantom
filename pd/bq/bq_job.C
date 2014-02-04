// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_job.H"

#include <pd/base/exception.H>

namespace pd {

struct bq_obj_t {
	string_t const name;
	log::backend_t const &log_backend;

	ref_t<bq_job_base_t> job;

	inline bq_obj_t(string_t const &_name, bq_job_base_t *_job) :
		name(_name), log_backend(log::handler_t::backend()), job(_job) { }

	inline ~bq_obj_t() { };
};

static void __run(void *_obj) {
	bq_obj_t *obj = (bq_obj_t *)_obj;

	log::handler_t handler(obj->name, &obj->log_backend);

	log_debug("bq_job start");

	safe_run(*obj->job, &bq_job_base_t::_run);

	log_debug("bq_job stop");

	delete obj;
}

void bq_job_base_t::run(bq_thr_t *bq_thr) {
	log::handler_base_t *current_log = log::handler_t::current();

	size_t name_size = current_log->print_label(NULL);

	string_t name = ({
		string_t::ctor_t ctor(name_size);
		current_log->print_label(&ctor);
		(string_t)ctor;
	});

	bq_obj_t *obj = new bq_obj_t(name, this);

	if(!bq_success(bq_cont_create(bq_thr, &__run, obj))) {
		delete obj;
		throw exception_sys_t(log::error, errno, "bq_cont_create: %m");
	}
}

} // namespace pd
