// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/fbind.H>
#include <pd/base/string.H>

#include <stdint.h>

#pragma GCC visibility push(default)

namespace pd {

typedef uintptr_t job_id_t;

class job_base_t : public ref_count_atomic_t {
protected:
	inline job_base_t() throw() { }

public:
	virtual ~job_base_t() throw() { }
	virtual void _run() const = 0;
	job_id_t run(string_t const &tname = string_t::empty);

	friend class ref_t<job_base_t>;
};

void job_wait(job_id_t);

template<typename x_t, typename ...args_t>
job_gen_t<void (x_t::*)(args_t...), job_base_t> job(void (x_t::*mfunc)(args_t...)) {
	return job_gen_t<void (x_t::*)(args_t...), job_base_t>(mfunc);
}

template<typename x_t, typename ...args_t>
job_gen_t<void (x_t::*)(args_t...) const, job_base_t> job(void (x_t::*mfunc)(args_t...) const) {
	return job_gen_t<void (x_t::*)(args_t...) const, job_base_t>(mfunc);
}

} // namespace pd

#pragma GCC visibility pop
