// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/io_benchmark/method_stream/logger.H>

#include <phantom/io_benchmark/method.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_benchmark { namespace method_stream {

class proto_t {
public:
	virtual bool reply_parse(
		in_t::ptr_t &ptr, in_segment_t const &request,
		unsigned int &res_code, stat_t &stat, logger_t::level_t &lev
	) const = 0;

	virtual size_t maxi() const throw() = 0;
	virtual descr_t const *descr(size_t ind) const throw() = 0;

protected:
	inline proto_t() throw() { }
	inline ~proto_t() throw() { }

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

}}} // namespace phantom::io_benchmark::method_stream

#pragma GCC visibility pop
