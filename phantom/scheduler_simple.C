// This file is part of the phantom program.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "scheduler.H"

#include <pd/bq/bq_heap.H>

#include <pd/base/size.H>
#include <pd/base/thr.H>
#include <pd/base/config.H>
#include <pd/base/config_enum.H>
#include <pd/base/exception.H>

namespace phantom {

class scheduler_simple_t : public scheduler_t {
	virtual bq_cont_activate_t &activate() throw();
	virtual bool switch_to(interval_t const &prio);
	virtual void init_ext();
	virtual void stat_ext(out_t &out, bool clear);
	virtual void fini_ext();

	class activate_t : public bq_cont_activate_t {
		virtual void operator()(bq_heap_t::item_t *item, bq_err_t err) {
			bq_cont_set_msg(item->cont, err);
			bq_cont_activate(item->cont);
		}
	public:
		inline activate_t() throw() { }
		inline ~activate_t() throw() { }
	};

	activate_t _activate;

public:
	struct config_t : scheduler_t::config_t {
		inline config_t() throw() : scheduler_t::config_t() { }
		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &ptr) const {
			scheduler_t::config_t::check(ptr);
		}
	};

	inline scheduler_simple_t(string_t const &name, config_t const &config) :
		scheduler_t(name, config), _activate() { }

	inline ~scheduler_simple_t() throw() { }
};

namespace scheduler_simple {
config_binding_sname(scheduler_simple_t);
config_binding_parent(scheduler_simple_t, scheduler_t, 1);
config_binding_ctor(scheduler_t, scheduler_simple_t);
}

bq_cont_activate_t &scheduler_simple_t::activate() throw() {
	return _activate;
}

bool scheduler_simple_t::switch_to(interval_t const &prio) {
	return bq_success(bq_thr()->switch_to(prio));
}

void scheduler_simple_t::init_ext() { }
void scheduler_simple_t::stat_ext(out_t &, bool) { }
void scheduler_simple_t::fini_ext() { }

} // namespace phantom
