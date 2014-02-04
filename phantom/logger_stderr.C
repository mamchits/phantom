// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "logger.H"

#include <pd/base/config.H>
#include <pd/base/mutex.H>
#include <pd/base/exception.H>
#include <pd/base/config_enum.H>

#include <unistd.h>

namespace phantom {

class logger_stderr_t : public logger_t {
public:
	struct config_t {
		config::enum_t<log::level_t> level;

		inline config_t() throw() : level(log::debug) { }
		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &) const { }
	};

	inline logger_stderr_t(string_t const &, config_t const &config) :
		mutex(), _level(config.level) { }

	inline ~logger_stderr_t() throw() { }

private:
	mutex_t mutable mutex;

	virtual void commit(iovec const *iov, size_t count) const throw();
	virtual log::level_t level() const throw();

	log::level_t _level;
};

namespace logger_stderr {
config_binding_sname(logger_stderr_t);
config_binding_value(logger_stderr_t, level);
config_binding_cast(logger_stderr_t, logger_t);
config_binding_ctor(logger_t, logger_stderr_t);
}

void logger_stderr_t::commit(iovec const *iov, size_t count) const throw() {
	// FIXME: write all iov
	mutex_guard_t guard(mutex);
	size_t __attribute__((unused)) n = writev(2, iov, count);
}

log::level_t logger_stderr_t::level() const throw() {
	return _level;
}

} // namespace phantom
