// This file is part of the phantom program.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "arena.H"

#include "jemalloc/malloc_arena.h"

#include <pd/base/thr.H>

namespace phantom {

class arena_t::impl_t {
	impl_t *next;

	arena_id_t arena_id;

	static thr::spinlock_t lock;
	static impl_t *list;

	inline impl_t() throw() : arena_id(arena_create()) { }
	inline ~impl_t() throw() { }

public:
	inline arena_id_t id() { return arena_id; }

	static inline impl_t *get() {
		impl_t *impl = ({
			impl_t *res = NULL;
			thr::spinlock_guard_t guard(lock);
			if(list) {
				res = list;
				list = list->next;
			}
			res;
		});

		return impl?: new impl_t;
	}

	static inline void put(impl_t *impl) {
		thr::spinlock_guard_t guard(lock);
		impl->next = list;
		list = impl;
	}
};

arena_t::impl_t *arena_t::impl_t::list = NULL;
thr::spinlock_t arena_t::impl_t::lock;

arena_t::arena_t() throw() : impl(impl_t::get()) { }

arena_t::~arena_t() throw() { impl_t::put(impl); }

void *arena_t::alloc(size_t size) throw() {
	return arena_alloc(impl->id(), size);
}

void *arena_t::realloc(void *optr, size_t size) throw() {
	return arena_realloc(impl->id(), optr, size);
}

void arena_t::free(void *ptr) throw() {
	arena_free(impl->id(), ptr);
}

} // namespace phantom
