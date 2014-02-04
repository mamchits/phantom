// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/bq/bq_thr.H>

#include <pd/base/fbind.H>

#pragma GCC visibility push(default)

namespace pd {

struct bq_job_base_t : public ref_count_atomic_t {
protected:
	inline bq_job_base_t() throw() { }

public:
	virtual ~bq_job_base_t() throw() { }
	virtual void _run() const = 0;
	void run(bq_thr_t *bq_thr);

	friend class ref_t<bq_job_base_t>;
};

template<typename x_t, typename ...args_t>
job_gen_t<void (x_t::*)(args_t...), bq_job_base_t> bq_job(void (x_t::*mfunc)(args_t...)) {
	return job_gen_t<void (x_t::*)(args_t...), bq_job_base_t>(mfunc);
}

template<typename x_t, typename ...args_t>
job_gen_t<void (x_t::*)(args_t...) const, bq_job_base_t> bq_job(void (x_t::*mfunc)(args_t...) const) {
	return job_gen_t<void (x_t::*)(args_t...) const, bq_job_base_t>(mfunc);
}

} // namespace pd

#pragma GCC visibility pop
