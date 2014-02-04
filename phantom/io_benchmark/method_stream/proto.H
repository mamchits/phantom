// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/io_benchmark/method_stream/logger.H>

#include <phantom/io_benchmark/method.H>

#include <phantom/shared.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_benchmark { namespace method_stream {

class proto_t : public shared_t {
public:
	virtual bool reply_parse(
		in_t::ptr_t &ptr, in_segment_t const &request,
		unsigned int &res_code, logger_t::level_t &lev
	) const = 0;

protected:
	inline proto_t(string_t const &_name) throw() : shared_t(_name) { }
	inline ~proto_t() throw() { }
};

}}} // namespace phantom::io_benchmark::method_stream

#pragma GCC visibility pop
