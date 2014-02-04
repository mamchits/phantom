// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/assert.H>
#include <pd/base/stat.H>

#include <stddef.h>

#pragma GCC visibility push(default)

namespace pd {

template<typename x_t>
class queue_t {
	size_t const maxcount;
	size_t count, head, tail;
	x_t *items;
	stat::mmcount_t &qcount;

public:
	inline void insert(x_t const &x) throw() {
		assert(count < maxcount);
		if(tail == maxcount) tail = 0;
		++count;
		items[tail++] = x;
		++qcount;
	}

	inline x_t remove() throw() {
		--qcount;
		if(head == maxcount) head = 0;
		assert(count > 0);
		x_t res = items[head];
		--count;
		items[head++] = x_t();
		return res;
	}

	inline queue_t(size_t _maxcount, stat::mmcount_t &_qcount) :
		maxcount(_maxcount), count(0), head(maxcount), tail(0), qcount(_qcount)  {
		items = new x_t[maxcount];
	}

	inline size_t get_count() const throw() { return count; }

	inline ~queue_t() throw() { assert(!count); delete [] items; }
};

} // namespace pd

#pragma GCC visibility pop
