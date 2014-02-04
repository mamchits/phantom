// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/log.H>

#include <errno.h>

#pragma GCC visibility push(default)

namespace pd {

class exception_t {
protected:
	inline exception_t() throw() { }
public:
	virtual ~exception_t() throw();
};

class exception_log_t : public exception_t {
	log::level_t log_level;
	log::aux_t *aux;

public:
	__format(3) exception_log_t(
		log::level_t _log_level, char const *fmt, ...
	) throw();

	virtual ~exception_log_t() throw();
};

class exception_sys_t : public exception_t {
	log::level_t log_level;
	log::aux_t *aux;

public:
	int const errno_val;

	__format(4) exception_sys_t(
		log::level_t _log_level, int _errno_val, char const *fmt, ...
	) throw() ;

	virtual ~exception_sys_t() throw();
};

template<typename x_t, typename y_t>
inline void safe_run(x_t &x, void (y_t::*func)(void)) throw() {
	try {
		(x.*func)();
	}
	catch(exception_t const &ex) {
	}
	catch(...) {
		log_error("unknown exception");
	}
}

template<typename x_t, typename y_t>
inline void safe_run(x_t const &x, void (y_t::*func)(void) const) throw() {
	try {
		(x.*func)();
	}
	catch(exception_t const &ex) {
	}
	catch(...) {
		log_error("unknown exception");
	}
}

} // namespace pd

#pragma GCC visibility pop
