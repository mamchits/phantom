// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/bq/bq_cont.H>

#include <pd/base/time.H>
#include <pd/base/assert.H>

#pragma GCC visibility push(default)

namespace pd {

class bq_heap_t {
public:
	struct item_t {
		bq_cont_t *cont;
		interval_t *timeout;
		timeval_t time_from;
		timeval_t time_to;
		bq_heap_t *heap;
		size_t ind;

		virtual void attach() throw() = 0;
		virtual void detach() throw() = 0;

		inline item_t(interval_t *_timeout) throw() :
			cont(bq_cont_current), timeout(_timeout),
			time_from(timeval_current()),
			time_to(timeout ? (time_from + *timeout) : timeval_never),
			heap(NULL), ind(0) { }

		inline ~item_t() throw() { assert(!ind); assert(!heap); }

	private:
		item_t(item_t const &);
		item_t &operator=(item_t const &);
	};

private:
	size_t count;
	size_t maxcount;
	item_t **items;

	inline item_t *get(size_t i) throw() {
		item_t *item = items[i];
		assert(item && item->ind == i);
		return item;
	}

	inline void put(size_t i, item_t *item) throw() {
		(items[i] = item)->ind = i;
	}

	static inline bool hcmp(item_t *i1, item_t *i2) throw() {
		return i1->time_to <= i2->time_to;
	}

	void place(item_t *item, size_t ind, bool flag) throw();

public:
	void insert(item_t *item);
	void remove(item_t *item) throw();

	void validate();

	inline item_t *head() throw() { return count ? items[1] : NULL; }

	inline bq_heap_t() : count(0), maxcount(0), items(NULL) { }

	~bq_heap_t() throw();

private: // don't use
	bq_heap_t(bq_heap_t const &);
	bq_heap_t &operator=(bq_heap_t const &);
};

class bq_cont_activate_t {
public:
	virtual void operator()(bq_heap_t::item_t *item, bq_err_t err) = 0;
};

} // namespace pd

#pragma GCC visibility pop
