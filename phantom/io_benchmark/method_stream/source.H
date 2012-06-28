// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/out.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_benchmark { namespace method_stream {

class source_t {
public:
	virtual bool get_request(in_segment_t &request, in_segment_t &tag) const = 0;
	virtual void init() = 0;
	virtual void stat(out_t &out, bool clear, bool hrr_flag) const = 0;
	virtual void fini() = 0;

protected:
	inline source_t() throw() { }
	inline ~source_t() throw() { }

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

}}} // namespace phantom::io_benchmark::method_stream

#pragma GCC visibility pop
