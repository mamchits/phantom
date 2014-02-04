// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/string.H>

#pragma GCC visibility push(default)

namespace pd { namespace config {

class syntax_t {
	syntax_t *next;
	string_t const type_name;
	void (*func)(out_t &);

	inline syntax_t(
		string_t const &_type_name, void (*_func)(out_t &)
	) : next(NULL), type_name(_type_name), func(_func) { };

	static syntax_t *list;

public:
	static void push(string_t const &, void (*func)(out_t &));

	static void proc(out_t &out);
	static void erase();
};

}} // namespace pd::config

#pragma GCC visibility pop
