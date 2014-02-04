// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "trace.H"
#include "out.H"
#include "mutex.H"

#include <malloc.h>
#include <dlfcn.h>

namespace pd {

void trace_setup(void const **ptrs, size_t n) {
	void const **frame = (void const **)__builtin_frame_address(0);
	void const **bound = frame + 4096;

	while(n) {
		*(ptrs++) = frame[1];
		--n;

		void const **nframe = (void const **)frame[0];
		if(nframe <= frame || nframe > bound)
			break;

		frame = nframe;
	}

	while(n) {
		*(ptrs++) = NULL;
		--n;
	}
}

void (*trace_addrinfo_print)(
	uintptr_t addr, uintptr_t addr_rel, char const *fname, out_t &out
);

static mutex_t trace_print_mutex;

void trace_print(void const *const *ptrs, size_t n, out_t &out) {
	mutex_guard_t guard(trace_print_mutex);

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
