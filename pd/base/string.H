// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/out.H>

#pragma GCC visibility push(default)

namespace pd {

class string_t : public in_segment_t {
	class __hidden page_t;

public:
	class ctor_t : public out_t {
		page_t *string_page;

		void setup_out(size_t _size, size_t _wpos);
		virtual void flush();

	public:
		ctor_t(size_t _size);
		~ctor_t() throw();

		void rollback(char c);

		template<typename x_t>
		inline ctor_t &print(x_t const &x, char const *fmt = NULL) {
			helper_t<x_t>::print(*this, x, fmt);
			return *this;
		}

		inline ctor_t &lf() {
			out_t::lf();
			return *this;
		}

		inline ctor_t &operator()(char c) {
			out_t::operator()(c);
			return *this;
		}

		inline ctor_t &operator()(in_segment_t const &str) {
			out_t::operator()(str);
			return *this;
		}

		inline ctor_t &operator()(in_t::ptr_t const &ptr, size_t len) {
			out_t::operator()(ptr, len);
			return *this;
		}

		inline ctor_t &operator()(str_t str) {
			out_t::operator()(str);
			return *this;
		}

		friend class string_t;
	};

private:
	inline void __check() {
		str_t str;
		if(!list->chunk(off_begin, str))
			fatal("string_t internal error");

		size_t len = size();

		if(str.size() < len) {
			ctor_t ctor(len);
			ctor(*this);

			*this = ctor;
		}

		assert(depth() == 0);
	}

public:
	inline string_t() : in_segment_t() { }

	string_t(ctor_t &ctor);

	explicit inline string_t(in_segment_t const &in_segment) :
		in_segment_t(in_segment) {
		if(size())
			__check();
	}

	inline string_t(ptr_t &ptr, size_t len) : in_segment_t(ptr, len) {
		if(size())
			__check();
	}

	inline string_t(size_t _off_begin, size_t _off_end, in_t::page_t *_page) :
		in_segment_t(_off_begin, _off_end, _page) { }

	inline ~string_t() throw() { }

	inline char const *ptr() const {
		if(!size())
			return NULL;

		str_t str;
		if(!list->chunk(off_begin, str))
			fatal("string_t internal error");

		assert(str.size() >= size());

		return str.ptr();
	}

	inline str_t str() const {
		return str_t(ptr(), size());
	}

	template<typename type_t>
	static inline cmp_t cmp(string_t const &str1, str_t const &str2) {
		return str_t::cmp<type_t>(str1.str(), str2);
	}

	template<typename type_t>
	static inline cmp_t cmp(string_t const &str1, string_t const &str2) {
		return str_t::cmp<type_t>(str1.str(), str2.str());
	}

	template<typename type_t>
	static inline bool cmp_eq(string_t const &str1, str_t const &str2) {
		return str_t::cmp_eq<type_t>(str1.str(), str2);
	}

	template<typename type_t>
	static inline bool cmp_eq(string_t const &str1, string_t const &str2) {
		return str_t::cmp_eq<type_t>(str1.str(), str2.str());
	}

	inline string_t copy() const { return *this; } // TODO: think about this

	inline string_t substring_tail(size_t off) const {
		return string_t(off_begin + off, off_end, list);
	}

	static ref_t<in_t::page_t> static_page_ref;
	static string_t illegal;
	static string_t empty;
};

inline string_t string(str_t str) { return string_t::ctor_t(str.size())(str); }

#define STRING(s) \
	(__builtin_constant_p(s) \
		? string_t( \
			(s) - (char const *)NULL, \
			(s) - (char const *)NULL + sizeof(s) - 1, \
			string_t::static_page_ref \
		) \
		: string_t::illegal)

#define MKCSTR(_var, _string) \
	char _var[(_string).size() + 1]; \
	{ \
		out_t(_var, sizeof(_var))((_string))('\0'); \
	}

} // namespace pd

#pragma GCC visibility pop
