// This file is part of the pd::base library.
// Copyright (C) 2012-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "in_str.H"
#include "exception.H"

namespace pd {

bool in_str_t::page_t::chunk(size_t off, str_t &str) const {
	if(off >= body.size())
		return false;

	str = str_t(body.ptr() + off, body.size() - off);
	return true;
}

unsigned int in_str_t::page_t::depth() const { return 0; }

bool in_str_t::page_t::optimize(in_segment_t &) const { return false; }

in_str_t::page_t::~page_t() throw() { }

bool in_str_t::do_expand() { return false; }

void __noreturn in_str_t::unexpected_end() const {
	throw exception_log_t(log::error | log::trace, "unexpected end of in_str_t");
}

} // namespace pd
