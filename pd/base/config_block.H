// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/config_helper.H>

#pragma GCC visibility push(default)

namespace pd { namespace config {

class block_t {
	virtual void parse_content(in_t::ptr_t &ptr) = 0;
	virtual void print_content(out_t &out, int off) const = 0;

protected:
	inline block_t() throw() { }
	inline ~block_t() throw() { }

public:
	void parse(in_t::ptr_t &ptr);
	inline void print(out_t &out, int off) const {
		out('{').lf();
		print_content(out, off + 2);
		print_off(out, off); out('}');
	}
};

}} // namespace pd::config

#pragma GCC visibility pop
