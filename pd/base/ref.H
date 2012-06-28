// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/assert.H>

#include <stddef.h>

#pragma GCC visibility push(default)

namespace pd {

template<typename x_t>
class ref_t {
	x_t *x;

public:
	inline ref_t(x_t *_x = NULL) : x(_x) {
		if(x && x->ref_count_inc()) fatal("bad ref count");
	}

	inline ref_t(ref_t const &r) : x(r.x) {
		if(x && x->ref_count_inc()) fatal("bad ref count");
	}

	inline ref_t &operator=(x_t *_x) {
		if(_x != x) {
			if(_x && _x->ref_count_inc()) fatal("bad ref count");
			if(x && x->ref_count_dec()) delete x;
			x = _x;
		}
		return *this;
	}

	inline ref_t &operator=(ref_t const &r) {
		return operator=(r.x);
	}

	inline ~ref_t() throw() {
		if(x && x->ref_count_dec()) delete x;
	}

	inline operator x_t *() const { return x; }
	inline x_t *operator->() const { return x; }
};

class ref_count_t {
	unsigned int counter;

protected:
	inline bool ref_count_inc() throw() { return !++counter; }
	inline bool ref_count_dec() throw() { return !--counter; }

	inline ref_count_t() throw() : counter(0) { }
	inline ref_count_t(ref_count_t const &) : counter(0) { }
	inline ref_count_t &operator=(ref_count_t const &) { return *this; }
	inline ~ref_count_t() throw() { assert(!counter); }
};

class ref_count_atomic_t {
	unsigned int counter;

protected:
	inline bool ref_count_inc() throw() {
		return !__sync_add_and_fetch(&counter, 1);
	}

	inline bool ref_count_dec() throw() {
		return !__sync_sub_and_fetch(&counter, 1);
	}

	inline ref_count_atomic_t() throw() : counter(0) { }
	inline ref_count_atomic_t(ref_count_atomic_t const &) : counter(0) { }
	inline ref_count_atomic_t &operator=(ref_count_atomic_t const &) { return *this; }
	inline ~ref_count_atomic_t() throw() { assert(!counter); }
};

} // namespace pd

#pragma GCC visibility pop
