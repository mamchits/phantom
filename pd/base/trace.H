// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <stddef.h>
#include <stdint.h>

#pragma GCC visibility push(default)

namespace pd {

class out_t;

void trace_setup(void const **ptrs, size_t n);
void trace_print(void const *const *ptrs, size_t n, out_t &out);

typedef void (*trace_addrinfo_print_t)(
	uintptr_t addr, uintptr_t addr_rel, char const *fname, out_t &out
);

extern trace_addrinfo_print_t trace_addrinfo_print;

template<unsigned int N>
class trace_t {
	void const *ptr[N];

public:
	inline trace_t() throw() { trace_setup(ptr, N); }

	inline ~trace_t() throw() { }

	inline void print(out_t &out) const { trace_print(ptr, N, out); }
};

} // namespace pd

#pragma GCC visibility pop
