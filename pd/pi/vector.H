// This file is part of the pd::pi library.
// Copyright (C) 2012-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/assert.H>

#pragma GCC visibility push(default)

template<typename x_t>
typename x_t::template _ptr_t<x_t> calc_ptr_type(x_t &);

#define for_each(p, some) for(typeof(calc_ptr_type(some)) p = (some); p; ++p)

#define for_each_next(p, p0) for(typeof(p0) p = p0; ++p;)

namespace pd {

template <typename x_t, typename count_t>
class vector_t {
protected:
	count_t count;
	x_t items[0];

	inline vector_t(count_t _count) throw() : count(_count) { }

public:
	template<typename _vector_t>
	class _ptr_t {
		static int nothing;
		typedef typeof(((_vector_t *)&nothing)->items[0]) _x_t;
		_x_t *ptr, *bound;

	public:
		inline _ptr_t(_vector_t &vector) throw() :
			ptr(vector.items), bound(ptr + vector.count) { }

		inline ~_ptr_t() throw() { }

		inline operator _x_t *() const throw() { return ptr; }
		inline _x_t *operator->() const throw() { return ptr; }
		inline _ptr_t &operator++() throw() { ++ptr; return *this; }
		inline _ptr_t operator++(int) throw() { _ptr_t p = *this; ++ptr; return p; }
		inline operator bool() const throw() { return ptr < bound; }
	};

	typedef _ptr_t<vector_t> ptr_t;
	typedef _ptr_t<vector_t const> c_ptr_t;
	typedef _ptr_t<vector_t volatile> v_ptr_t;
	typedef _ptr_t<vector_t const volatile> cv_ptr_t;

	inline count_t _count() const { return count; }

	inline x_t &operator[](count_t i) throw() {
		assert(i < count);
		return items[i];
	}

	inline x_t const &operator[](count_t i) const throw() {
		assert(i < count);
		return items[i];
	}

	inline x_t volatile &operator[](count_t i) volatile throw() {
		assert(i < count);
		return items[i];
	}

	inline x_t const volatile &operator[](count_t i) const volatile throw() {
		assert(i < count);
		return items[i];
	}

	inline vector_t() throw() : count(0) { }
};

} // namespace pd

#pragma GCC visibility pop
