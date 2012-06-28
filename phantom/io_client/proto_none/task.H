// This file is part of the phantom::io_client::proto_none module.
// Copyright (C) 2010-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/bq/bq_cond.H>

#include <pd/base/ref.H>
#include <pd/base/out.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_client { namespace proto_none {

class task_t : public ref_count_atomic_t {
	timeval_t begin_time;
	timeval_t complete_time;
	enum { processed, ready, canceled } status;
	bq_cond_t cond;

public:
	virtual bool parse(in_t::ptr_t &ptr) = 0;
	virtual void print(out_t &out) const = 0;
	virtual void clear() = 0;

	inline void set_begin() {
		timeval_t now = timeval_current();

		bq_cond_guard_t guard(cond);
		if(!begin_time.is_real()) begin_time = now;
	}

	inline void set_ready() {
		timeval_t now = timeval_current();
		assert(begin_time.is_real());

		bq_cond_guard_t guard(cond);
		status = ready;
		complete_time = now;
		cond.send();
	}

	inline void cancel() {
		bq_cond_guard_t guard(cond);
		status = canceled;
		cond.send();
	}

	inline bool active() {
		return status == processed;
	}

	inline bool wait(interval_t *timeout, interval_t *wtime) {
		bq_cond_guard_t guard(cond);

		while(true) {
			if(status != processed) {
				bool res = (status == ready);
				if(res) {
					assert(complete_time != timeval_never);
					if(wtime) *wtime = (complete_time - begin_time);
				}
				return res;
			}

			if(!bq_success(cond.wait(timeout)))
				return false;
		}
	}

	static inline bool hcmp(task_t const *task1, task_t const *task2) {
		return task1->begin_time <= task2->begin_time;
	}

protected:
	inline task_t() :
		begin_time(timeval_never), complete_time(timeval_never),
		status(processed), cond() { }

	virtual ~task_t() throw();

	friend class ref_t<task_t>;
};

}}} // namespace phantom::io_client::proto_none

#pragma GCC visibility pop
