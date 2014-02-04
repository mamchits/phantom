// This file is part of the pd::base library.
// Copyright (C) 2012-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/in.H>

#pragma GCC visibility push(default)

namespace pd {

class in_str_t : public in_t {
	class page_t : public in_t::page_t {
		str_t body;

		virtual bool chunk(size_t off, str_t &str) const;
		virtual unsigned int depth() const;
		virtual bool optimize(in_segment_t &segment) const;

		inline page_t(str_t const &_body) throw() : in_t::page_t(), body(_body) { }
		virtual ~page_t() throw();

		inline void *operator new(size_t size, void *ptr) {
			assert(size == sizeof(page_t));
			return ptr;
		}

		inline void operator delete(void *) throw() { }

		friend class in_str_t;
	};

	virtual bool do_expand();
	virtual void __noreturn unexpected_end() const;

	char page_buf[sizeof(page_t)] __aligned(__alignof__(page_t));

public:
	inline in_str_t(str_t const &_str) : in_t() {
		list = new (page_buf) page_t(_str);
		off_end += _str.size();
	}

	inline ~in_str_t() throw() { list = NULL; }
};

} // namespace pd

#pragma GCC visibility pop
