// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/shared.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_benchmark { namespace method_stream {

class source_t : public shared_t {
public:
	virtual bool get_request(in_segment_t &request, in_segment_t &tag) const = 0;

protected:
	inline source_t(string_t const &name) throw() : shared_t(name) { }
	inline ~source_t() throw() { }
};

}}} // namespace phantom::io_benchmark::method_stream

#pragma GCC visibility pop
