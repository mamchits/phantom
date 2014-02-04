// This file is part of the phantom program.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/stat_ctx.H>
#include <pd/base/log.H>

#pragma GCC visibility push(default)

namespace phantom {
class shared_t {
protected:
	string_t const *master;

	virtual void do_init() = 0;
	virtual void do_run() const = 0;
	virtual void do_stat_print() const = 0;
	virtual void do_fini() = 0;

public:
	string_t const name;

	inline void init(string_t const &_master) {
		if(!master) {
			master = &_master;
			log::handler_t handler(name);
			do_init();
		}
	}

	inline void run(string_t const &_master) const {
		if(&_master == master) {
			log::handler_t handler(name);
			do_run();
		}
	}

	inline void stat_print(string_t const &_master) const {
		stat::ctx_t ctx(name.str());
		if(master) {
			if(&_master == master) {
				do_stat_print();
			}
			else
				stat::link(*master);
		}
	}

	inline void fini(string_t const &_master) {
		if(&_master == master) {
			log::handler_t handler(name);
			do_fini();
		}
	}

protected:
	inline shared_t(string_t const &_name) throw() :
		master(NULL), name(_name) { }

	inline ~shared_t() throw() { }

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

} // namespace phantom

#pragma GCC visibility pop
