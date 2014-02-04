// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "job.H"
#include "thr.H"
#include "exception.H"

#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>

namespace pd {

struct obj_t {
	string_t const name;
	string_t const tname;
	log::backend_t const &log_backend;

	ref_t<job_base_t> job;

	inline obj_t(
		string_t const &_name, string_t const &_tname, job_base_t *_job
	) :
		name(_name), tname(_tname),
		log_backend(log::handler_t::backend()), job(_job) { }

	inline ~obj_t() { };
};

static void *__run(void *_obj) {
	obj_t *obj = (obj_t *)_obj;

	if(obj->tname) {
		MKCSTR(_tname, obj->tname);
		::prctl(PR_SET_NAME, _tname);
	}

	thr::id = ::syscall(SYS_gettid);

	log::handler_t log_handler(obj->name, &obj->log_backend);

	//log_debug("job start");

	safe_run(*obj->job, &job_base_t::_run);

	//log_debug("job stop");

	delete obj;

	thr::id = 0;

	return NULL;
}

job_id_t job_base_t::run(string_t const &tname) {
	log::handler_base_t *handler= log::handler_t::current();

	size_t name_size = handler->print_label(NULL);

	string_t name = ({
		string_t::ctor_t ctor(name_size);
		handler->print_label(&ctor);
		(string_t)ctor;
	});

	obj_t *obj = new obj_t(name, tname, this);

	pthread_t pthread;
	int err = pthread_create(&pthread, NULL, &__run, obj);

	if(err) {
		delete obj;
		throw exception_sys_t(log::error, err, "pthread_create: %m");
	}

	return pthread;
}

void job_wait(job_id_t id) {
	errno = pthread_join(id, NULL);

	if(errno)
		log_error("pthread_join: %m");
}

} // namespace pd
