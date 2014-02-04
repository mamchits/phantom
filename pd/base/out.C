// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "out.H"
#include "exception.H"

#include <unistd.h>

namespace pd {

void out_t::flush() {
	throw exception_log_t(log::error | log::trace, "buffer overflow");
}

out_t &out_t::ctl(int) {
	return *this;
}

out_t &out_t::sendfile(int from_fd, off_t &_offset, size_t &_len) {
	off_t offset = _offset;
	size_t len = _len;

	while(len) {
		wcheck();
		size_t tlen = min((((rpos > wpos) ? rpos : size) - wpos), len);

		ssize_t res = ::pread(from_fd, data + wpos, tlen, offset);

		if(res < 0)
			throw exception_sys_t(log::error, errno, "pread: %m");

		if(res == 0) break;

		minimize(tlen, res);

		wpos += tlen;
		offset += tlen;
		len -= tlen;
	}

	_offset = offset;
	_len = len;

	return *this;
}

static inline void print_pointer(out_t &out, void const *ptr, char const *fmt) {
	char const *digits = "0123456789ABCDEF";

	if(fmt) {
		while(*fmt) {
			if(*fmt == 'x') { digits="0123456789abcdef"; }
			++fmt;
		}
	}

	if(ptr) {
		uintptr_t val = ({
			union {
				void const *ptr;
				uintptr_t val;
			} u;

			u.ptr = ptr;
			u.val;
		});

		uintptr_t mask = ~((~(uintptr_t)0) >> 4);
		unsigned int shift = (sizeof(ptr) * 8);
		do {
			shift -= 4;
			out(digits[(val & mask) >> shift]);
			mask >>= 4;
		} while(mask);
	}
	else
		out(CSTR("NULL"));
}

template<>
void out_t::helper_t<void const *>::print(out_t &out, void const *const &x, char const *fmt) {
	print_pointer(out, x, fmt);
}

template<>
void out_t::helper_t<void *>::print(out_t &out, void *const &x, char const *fmt) {
	print_pointer(out, x, fmt);
}

} // namespace pd
