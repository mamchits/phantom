// This file is part of the pd::debug library.
// Copyright (C) 2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/out.H>

#include <stdint.h>

#pragma GCC visibility push(default)

namespace pd { namespace debug {

void trace_addrinfo_print_bfd(
	uintptr_t addr, uintptr_t addr_rel, char const *fname, out_t &out
);

}} // namespace pd::debug

#pragma GCC visibility pop
