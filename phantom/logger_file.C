// This file is part of the phantom program.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "logger.H"
#include "io_logger_file.H"

#include <pd/base/config_enum.H>

namespace phantom {

class logger_file_t : public logger_t, io_logger_file_t {
	log::level_t _level;

	virtual void commit(iovec const *iov, size_t count) const throw();
	virtual log::level_t level() const throw();

public:
	struct config_t : io_logger_file_t::config_t {
		config::enum_t<log::level_t> level;

		inline config_t() throw() :
			io_logger_file_t::config_t(), level(log::debug) { }

		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &ptr) const {
			io_logger_file_t::config_t::check(ptr);
		}
	};

	inline logger_file_t(string_t const &name, config_t const &config) :
		logger_t(),
		io_logger_file_t(name, config),
		_level(config.level) { }

	inline ~logger_file_t() throw() { }
};

namespace logger_file {
config_binding_sname(logger_file_t);
config_binding_value(logger_file_t, level);
config_binding_parent(logger_file_t, io_logger_file_t, 1);
config_binding_ctor(logger_t, logger_file_t);
}

void logger_file_t::commit(iovec const *iov, size_t count) const throw() {
	writev(iov, count);
}

log::level_t logger_file_t::level() const throw() {
	return _level;
}

} // namespace phantom
