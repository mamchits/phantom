// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
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
	virtual void log() const = 0;
	virtual str_t msg() const = 0;
	virtual ~exception_t() throw();
};

class exception_log_t : public exception_t {
	log::level_t log_level;

protected:
	log::aux_t *aux;
	char buf[256];

	inline exception_log_t(log::level_t _log_level) throw() :
		log_level(_log_level), aux(NULL) { }

public:
	__format(3) exception_log_t(
		log::level_t _log_level, char const *fmt, ...
	) throw();

	virtual void log() const;
	virtual str_t msg() const;
	virtual ~exception_log_t() throw();
};

class exception_sys_t : public exception_log_t {
public:
	int const errno_val;

	__format(4) exception_sys_t(
		log::level_t _log_level, int _errno_val, char const *fmt, ...
	) throw() ;
};

template<typename x_t, typename y_t>
inline void safe_run(x_t &x, void (y_t::*func)(void)) throw() {
	try {
		(x.*func)();
	}
	catch(exception_t const &ex) {
		ex.log();
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
		ex.log();
	}
	catch(...) {
		log_error("unknown exception");
	}
}

} // namespace pd

#pragma GCC visibility pop
