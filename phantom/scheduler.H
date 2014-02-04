// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/obj.H>

#include <pd/bq/bq_thr.H>
#include <pd/base/size.H>
#include <pd/base/time.H>
#include <pd/base/config.H>
#include <pd/base/config_enum.H>

#include <sched.h>

#pragma GCC visibility push(default)

namespace phantom {

class scheduler_t : public obj_t {
public:
	enum policy_t {
		other = SCHED_OTHER,
		fifo = SCHED_FIFO,
		rr = SCHED_RR,
		batch = SCHED_BATCH
	};

protected:
	string_t tname;

private:
	virtual bq_post_activate_t *post_activate() = 0;

	virtual void init_ext() = 0;
	virtual void stat_print_ext() const = 0;
	virtual void fini_ext() = 0;

	size_t threads;
	size_t event_buf_size;
	interval_t timeout_prec;

	policy_t policy;
	int priority;
	bool need_set_priority;

	class ind_t {
		size_t maxval;
		size_t val;
		spinlock_t spinlock;

	public:
		inline ind_t(size_t _maxval) throw() :
			maxval(_maxval), val(0), spinlock() { }

		inline size_t nextval() {
			spinlock_guard_t _guard(spinlock);
			if(val >= maxval) val = 0;
			return val++;
		}
	};

	bq_cont_count_t cont_count;
	bq_thr_t *bq_thrs;
	ind_t mutable ind;

	virtual void init();
	virtual void exec() const;
	virtual void stat_print() const;
	virtual void fini();

public:
	inline size_t bq_n() const { return threads; }
	inline bq_thr_t *bq_thr(size_t i) const { return &bq_thrs[i % threads]; }
	inline bq_thr_t *bq_thr() const { return &bq_thrs[ind.nextval()]; }

	virtual bool switch_to(interval_t const &prio) = 0;

	struct config_t {
		config::enum_t<policy_t> policy; policy_t policy_orig;
		int priority, priority_orig;
		sizeval_t threads;
		string_t tname;
		sizeval_t limit;
		sizeval_t event_buf_size;
		interval_t timeout_prec;

		config_t() throw();
		void check(in_t::ptr_t const &ptr) const;
		inline ~config_t() throw() { }
	};

protected:
	scheduler_t(string_t const &_name, config_t const &config);
	~scheduler_t() throw();

	static bool bq_work;
};

} // namespace phantom

#pragma GCC visibility pop
