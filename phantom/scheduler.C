// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "scheduler.H"

#include <pd/base/exception.H>

namespace phantom {

scheduler_t::config_t::config_t() throw() :
	policy((policy_t)sched_getscheduler(0)), policy_orig(policy),
	priority(({ sched_param p; sched_getparam(0, &p); p.sched_priority; })),
	priority_orig(priority),
	threads(1), limit(sizeval::unlimited), event_buf_size(20),
	timeout_prec(10 * interval::millisecond) { }

void scheduler_t::config_t::check(in_t::ptr_t const &ptr) const {
	if(!threads)
		config::error(ptr, "threads must be a positive number");

	if(threads > 128)
		config::error(ptr, "threads is too big");
}

namespace scheduler {
config_binding_sname(scheduler_t);
config_binding_value(scheduler_t, threads);
config_binding_value(scheduler_t, limit);
config_binding_value(scheduler_t, event_buf_size);
config_binding_value(scheduler_t, timeout_prec);
config_binding_value(scheduler_t, tname);
config_binding_value(scheduler_t, policy);
config_binding_value(scheduler_t, priority);

namespace policy {
config_enum_internal_sname(scheduler_t, policy_t);
config_enum_internal_value(scheduler_t, policy_t, other);
config_enum_internal_value(scheduler_t, policy_t, fifo);
config_enum_internal_value(scheduler_t, policy_t, rr);
config_enum_internal_value(scheduler_t, policy_t, batch);
}}

scheduler_t::scheduler_t(string_t const &name, config_t const &config) :
	obj_t(name), tname(config.tname), threads(config.threads),
	event_buf_size(config.event_buf_size), timeout_prec(config.timeout_prec),
	policy(config.policy), priority(config.priority),
	need_set_priority(
		(config.policy != config.policy_orig) ||
		(config.priority != config.priority_orig)
	),
	cont_count(config.limit), bq_thrs(NULL), ind(threads) {

	bq_thrs = new bq_thr_t[threads];
}

scheduler_t::~scheduler_t() throw() {
	delete [] bq_thrs;
}

void scheduler_t::init() {
	sched_param sp = { priority };

	size_t i = 0;
	try {
		char const *fmt = log::number_fmt(threads);

		for(; i < threads; ++i) {
			string_t name = string_t::ctor_t(5).print(i, fmt);
			log::handler_t handler(name);

			bq_thrs[i].init(
				event_buf_size, timeout_prec, cont_count, tname, post_activate()
			);

			if(need_set_priority)
				if(sched_setscheduler(bq_thrs[i].get_tid(), policy, &sp) < 0)
					throw exception_sys_t(log::error, errno, "sched_setscheduler: %m");
		}
	}
	catch(...) {
		bq_thr_t::stop();

		bq_thrs[i].fini();

		while(i)
			bq_thrs[--i].fini();

		throw;
	}

	init_ext();
}

void scheduler_t::exec() const { }

void scheduler_t::stat_print() const {
	{
		stat::ctx_t ctx(CSTR("bqs"), 1);
		for(size_t i = 0; i < threads; ++i)
			bq_thrs[i].stat_print();
	}

	stat_print_ext();
}

void scheduler_t::fini() {
	for(size_t i = threads; i;)
		bq_thrs[--i].fini();

	fini_ext();
}

} // namespace phantom
