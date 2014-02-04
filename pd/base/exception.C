// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "exception.H"
#include "trace.H"

#include <stdio.h>

namespace pd {

exception_t::~exception_t() throw() { }

class aux_exception_t : public log::aux_t {
	trace_t<16> trace;

	virtual void print(out_t &out) const;
public:
	inline aux_exception_t() throw() { }
	virtual ~aux_exception_t() throw();
};

aux_exception_t::~aux_exception_t() throw() { }
void aux_exception_t::print(out_t &out) const { trace.print(out); }

exception_log_t::exception_log_t(
	log::level_t _log_level, char const *fmt, ...
) throw() :
	log_level(_log_level & ~log::trace), aux(NULL) {

	log::handler_base_t *handler = log::handler_t::current();

	if(handler->level() > _log_level)
		return;

	if(trace_addrinfo_print || (_log_level & log::trace))
		aux = new aux_exception_t;

	va_list args;
	va_start(args, fmt);
	handler->vput(log_level, aux, fmt, args);
	va_end(args);
}

exception_log_t::~exception_log_t() throw() { delete aux; }

exception_sys_t::exception_sys_t(
	log::level_t _log_level, int _errno_val, char const *fmt, ...
) throw() :
	log_level(_log_level & ~log::trace), aux(NULL), errno_val(_errno_val) {

	log::handler_base_t *handler = log::handler_t::current();

	if(handler->level() > _log_level)
		return;

	if(trace_addrinfo_print || (_log_level & log::trace))
		aux = new aux_exception_t;

	errno = _errno_val;

	va_list args;
	va_start(args, fmt);
	handler->vput(log_level, aux, fmt, args);
	va_end(args);
}

exception_sys_t::~exception_sys_t() throw() { delete aux; }

} // namespace pd
