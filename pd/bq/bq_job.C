// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_job.H"

#include <pd/base/exception.H>
#include <pd/base/trace.H>

#include <sys/prctl.h>

namespace pd {

static void __run_bq(void *_job) {
	bq_job_base_t *job = (bq_job_base_t *)_job;

	log::handler_default_t log_handler(job->name, &job->log_backend);

	safe_run(*job, &bq_job_base_t::_run);

	delete job;
}

static void *__run_thr(void *_job) {
	bq_job_base_t *job = (bq_job_base_t *)_job;

	MKCSTR(_name, job->name);

	prctl(PR_SET_NAME, _name);

	log::handler_default_t log_handler(job->name, &job->log_backend);

	safe_run(*job, &bq_job_base_t::_run);

	delete job;

	return NULL;
}

void bq_job_base_t::__create(bq_thr_t *bq_thr) {
	if(!bq_success(bq_cont_create(bq_thr, &__run_bq, this))) {
		delete this;
		throw exception_sys_t(log::error, errno, "bq_cont_create: %m");
	}
}

void bq_job_base_t::__create(pthread_t *pthread, pthread_attr_t *attr) {
	int err = pthread_create(pthread, attr, &__run_thr, this);
	if(err) {
		delete this;
		throw exception_sys_t(log::error, err, "pthread_create: %m");
	}
}

bq_job_base_t::~bq_job_base_t() throw() { }

} // namespace pd
