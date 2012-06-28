// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "exception.H"
#include "trace.H"

#include <stdio.h>

namespace pd {

exception_t::~exception_t() throw() { }

#define fmt_print(fmt, buf) \
	do { \
		va_list vargs; \
		va_start(vargs, fmt); \
		vsnprintf(buf, sizeof(buf), fmt, vargs); \
		va_end(vargs); \
		buf[sizeof(buf) - 1] = '\0'; \
	} while(0)

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
) throw() : log_level(_log_level & ~log::trace) {

	aux = trace_addrinfo_print || (_log_level & log::trace)
		? new aux_exception_t
		: NULL
	;

	fmt_print(fmt, buf);
}

void exception_log_t::log() const {
	log_put(log_level, aux, "%s", buf);
}

str_t exception_log_t::msg() const {
	return str_t(buf, strlen(buf));
}

exception_log_t::~exception_log_t() throw() { if(aux) delete aux; }

exception_sys_t::exception_sys_t(
	log::level_t _log_level, int _errno_val, char const *fmt, ...
) throw() : exception_log_t(_log_level), errno_val(_errno_val) {

	aux = trace_addrinfo_print ? new aux_exception_t : NULL;

	errno = _errno_val;
	fmt_print(fmt, buf);
}

} // namespace pd
