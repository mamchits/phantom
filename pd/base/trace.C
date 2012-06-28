// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "trace.H"
#include "out.H"
#include "thr.H"

#include <malloc.h>
#include <dlfcn.h>

namespace pd {

void trace_setup(void const **ptrs, size_t n) {
	unsigned int i = 0;

	for(
		void const *const *frame = (void **)__builtin_frame_address(0);
		frame && i < n;
		frame = (const void *const *)(frame[0]), ++i
	)
		*(ptrs++) = frame[1];

	for(; i < n; ++i) *(ptrs++) = NULL;
}

void (*trace_addrinfo_print)(
	uintptr_t addr, uintptr_t addr_rel, char const *fname, out_t &out
);

static thr::mutex_t trace_print_mutex;

void trace_print(void const *const *ptrs, size_t n, out_t &out) {
	thr::mutex_guard_t guard(trace_print_mutex);

	for(unsigned int i = 0; i < n; i++) {
		uintptr_t addr = (uintptr_t)*(ptrs++);
		if(!addr) break;

		Dl_info dl_info = { NULL, NULL, NULL, NULL };

		if(dladdr((void const *)addr, &dl_info)) {
			uintptr_t addr_rel = addr - (uintptr_t)dl_info.dli_fbase;

			out
				(' ')(str_t(dl_info.dli_fname, strlen(dl_info.dli_fname)))
				(' ').print(addr, "x")
				(' ').print(addr_rel, "x")
				.lf()
			;

			if(trace_addrinfo_print) {
				(*trace_addrinfo_print)(addr, addr_rel, dl_info.dli_fname, out);
				out.lf();
			}
		}
		// out.flush_all();
	}
}

} // namespace pd
