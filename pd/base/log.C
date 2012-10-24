// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "log.H"
#include "thr.H"
#include "time.H"
#include "config_enum.H"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

namespace pd { namespace log {

class backend_default_t : public backend_t {
	virtual void commit(iovec const *iov, size_t count) const throw();
	virtual level_t level() const throw();
public:
	inline backend_default_t() throw() { }
	inline ~backend_default_t() throw() { }
};

level_t backend_default_t::level() const throw() { return debug; }

void backend_default_t::commit(iovec const *iov, size_t count) const throw() {
	// FIXME: write all iov
	size_t __attribute__((unused)) n = writev(2, iov, count);
}

static backend_default_t __init_priority(101) const backend_deafult;

aux_t::~aux_t() throw() { }

static __init_priority(110) handler_default_t const handler_default(STRING("*"), &backend_deafult);

static __thread handler_t const *handler_current = NULL;

static handler_t const *default_get() {
	return handler_current ?: &handler_default;
}

static void default_set(handler_t const *handler) {
	handler_current = handler;
}

static void (*set_func)(handler_t const *) = &default_set;
static handler_t const *(*get_func)() = &default_get;

void handler_t::init(
	void (*_set)(handler_t const *), handler_t const *(*_get)()
) throw() {
	set_func = _set ?: &default_set;
	get_func = _get ?: &default_get;
}

handler_t const *handler_t::get() throw() {
	try {
		return (*get_func)() ?: &handler_default;
	}
	catch(...) {
		return &handler_default;
	}
}

void handler_t::set(handler_t const *handler) throw() {
	try {
		(*set_func)(handler);
	}
	catch(...) { }
}

static string_t const levels[] = {
	STRING("[debug]"),
	STRING("[info]"),
	STRING("[warning]"),
	STRING("[error]")
};

void handler_default_t::vput(
	level_t level, aux_t const *aux, char const *format, va_list args
) const throw() {
	if(backend.level() > level)
		return;

	int __errno = errno; // Save errno for proper %m format handling

	char buf[1024];

	size_t len = ({
		out_t out(buf, sizeof(buf));

		try {
			out.print(timeval_current(), "+dz.")(' ')(levels[level])(' ')(label)(' ');
		}
		catch(...) { }

		out.used();
	});

	if(len < sizeof(buf)) {
		errno = __errno;
		len += (size_t)vsnprintf(buf + len, sizeof(buf) - len, format, args);
	}

	if(len >= sizeof(buf)) len = sizeof(buf) - 1;
	buf[len++] = '\n';

	if(aux) {
		char _buf[40960];
		size_t _len = ({
			out_t _out(_buf, sizeof(_buf));
			try {
				aux->print(_out);
			}
			catch(...) { }
			_out.used();
		});

		iovec vec[2] = {
			(iovec){ buf, len },
			(iovec){ _buf, _len },
		};

		backend.commit(vec, 2);
	}
	else
		backend.commit(buf, len);
}

config_enum_internal_sname(log, level_t);
config_enum_internal_value(log, level_t, debug);
config_enum_internal_value(log, level_t, info);
config_enum_internal_value(log, level_t, warning);
config_enum_internal_value(log, level_t, error);

}} // namespace pd::log
