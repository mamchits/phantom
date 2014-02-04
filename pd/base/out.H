// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/in.H>

#pragma GCC visibility push(default)

namespace pd {

class out_t {
protected:
	char *data;
	size_t size;
	size_t wpos, rpos;

	virtual void flush();

	inline void wcheck() {
		if(wpos == rpos) flush();
		if(wpos == size) wpos = 0;
	}

public:
	virtual out_t &ctl(int i);

	inline out_t &operator()(str_t const &_str) {
		str_t::ptr_t ptr = _str;

		while(ptr) {
			wcheck();
			size_t tlen = min((((rpos > wpos) ? rpos : size) - wpos), ptr.size());

			memcpy(data + wpos, ptr(), tlen);
			wpos += tlen;
			ptr += tlen;
		}
		return *this;
	}

	inline size_t used() const {
		return ((rpos >= wpos) ? size : 0) + wpos - rpos;
	}

	inline out_t &operator()(in_t::ptr_t const &ptr, size_t len) {
		in_t::ptr_t p = ptr;

		while(len) {
			str_t str = p.__chunk();

			str.truncate(len);

			(*this)(str);

			size_t clen = str.size();

			len -= clen;
			p += clen;
		}

		return *this;
	}

	inline out_t &operator()(in_segment_t const &str) {
		in_t::ptr_t p = str;

		return (*this)(p, str.size());
	}

	inline out_t &operator()(char c) {
		wcheck(); data[wpos++] = c; return *this;
	}

	inline out_t &lf() { return (*this)('\n'); }
	inline out_t &crlf() { return (*this)('\r')('\n'); }

	virtual out_t &sendfile(int fd, off_t &offset, size_t &size);

	inline void flush_all() {
		while(rpos < size || wpos > 0) flush();
	}

	inline out_t(char *_data, size_t _size) throw() :
		data(_data), size(_size), wpos(0), rpos(size) { }

	inline ~out_t() throw() { }

	out_t(out_t const &) = delete;
	out_t &operator=(out_t const &) = delete;

	template<typename x_t>
	struct helper_t {
		static void print(out_t &, x_t const &, char const *fmt);
	};

	template<typename x_t>
	inline out_t &print(x_t const &x, char const *fmt = NULL) {
		helper_t<x_t>::print(*this, x, fmt);
		return *this;
	}

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

} // namespace pd

#pragma GCC visibility pop
