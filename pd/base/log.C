// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "log.H"
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

static backend_default_t __init_priority(101) backend_deafult;

aux_t::~aux_t() throw() { }

static __init_priority(102)
	handler_t handler_default(STRING("*"), &backend_deafult, true);

static __thread handler_base_t *handler_current = NULL;

static handler_base_t *&default_current_funct() throw() {
	if(!handler_current)
		handler_current = &handler_default;

	return handler_current;
}

static handler_base_t::current_funct_t current_funct = &default_current_funct;

handler_base_t::current_funct_t
	handler_base_t::setup(current_funct_t _current_funct) throw()
{
	current_funct_t res = current_funct;
	current_funct = _current_funct ?: &default_current_funct;
	return res;
}

handler_base_t *&handler_base_t::current() throw() {
	try {
		handler_base_t *&cur = (*current_funct)();
		if(!cur) cur = &handler_default;

		return cur;
	}
	catch(...) { }

	return default_current_funct();
}

static string_t const levels[] = {
	STRING("[debug]"),
	STRING("[info]"),
	STRING("[warning]"),
	STRING("[error]")
};

size_t handler_base_t::print_label(out_t *out, bool sep) const {
	if(this == &handler_default)
		return 0;

	if(root /* && sep */)
		return 0;

	size_t res = root ? 0 : prev->print_label(out, true);

	if(out) {
		(*out)(_label);
		if(sep) (*out)(' ');
	}

	return res + _label.size() + 1;
}

void handler_t::vput(
	level_t level, aux_t const *aux, char const *format, va_list args
) const throw() {
	if(_backend.level() > level)
		return;

	int __errno = errno; // Save errno for proper %m format handling

	char buf[1024];

	size_t len = ({
		out_t out(buf, sizeof(buf));

		try {
			out.print(timeval::current(), "+dz.")(' ')(levels[level])(' ')('[');

			print_label(&out);

			out(']')(' ');
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

		_backend.commit(vec, 2);
	}
	else
		_backend.commit(buf, len);
}

config_enum_internal_sname(log, level_t);
config_enum_internal_value(log, level_t, debug);
config_enum_internal_value(log, level_t, info);
config_enum_internal_value(log, level_t, warning);
config_enum_internal_value(log, level_t, error);

}} // namespace pd::log
