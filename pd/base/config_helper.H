// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/string.H>

#pragma GCC visibility push(default)

namespace pd { namespace config {

extern bool check;

string_t setup(int _argc, char *_argv[], char *envp[]);

template<typename x_t>
struct helper_t {
	static void parse(in_t::ptr_t &ptr, x_t &x);
	static void print(out_t &out, int off, x_t const &x);
	static void syntax(out_t &out);
};

string_t parse_name(in_t::ptr_t &ptr);
char skip_space(in_t::ptr_t &ptr);
void print_off(out_t &out, int off);

struct exception_t  {
	in_t::ptr_t ptr;

	inline exception_t(in_t::ptr_t const &_ptr) throw() : ptr(_ptr) { }
};

void __noreturn error(in_t::ptr_t const &ptr, char const *msg);
void report_position(string_t const &name, in_t const &in, in_t::ptr_t const &ptr);
void report_obj(string_t const &obj_name);

}} // namespace pd::config

#pragma GCC visibility pop
