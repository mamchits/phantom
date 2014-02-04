// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/obj.H>

#include <pd/base/config.H>

#pragma GCC visibility push(default)

namespace phantom {

class scheduler_t;

class io_t : public obj_t {
	void do_exec() const;
	virtual void exec() const;
	virtual void run() const = 0;

public:
	struct config_t {
		config::objptr_t<scheduler_t> scheduler;

		inline config_t() throw() : scheduler() { }

		inline void check(in_t::ptr_t const &ptr) const {
			if(!scheduler)
				config::error(ptr, "scheduler is required");
		}

		inline ~config_t() throw() { }
	};

	static unsigned int active_count;

protected:
	scheduler_t const &scheduler;

	bool const active;

	inline io_t(
		string_t const &name, config_t const &config, bool _active = false
	) throw() :
		obj_t(name), scheduler(*config.scheduler), active(_active) {

		if(active)
			++active_count;
	}

	inline ~io_t() throw() { }
};

} // namespace phantom

#pragma GCC visibility pop
