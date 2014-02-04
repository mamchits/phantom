// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "exception.H"

#include <malloc.h>

#ifndef DEBUG_ALLOC

#pragma GCC visibility push(default)

void *operator new(size_t sz) {
	void *ptr = ::malloc(sz);

	if(!ptr && sz)
		throw pd::exception_sys_t(pd::log::error, ENOMEM, "malloc: %m");

	return ptr;
}

void operator delete(void *ptr) throw() { ::free(ptr); }

void *operator new[](size_t sz) {
	void *ptr = ::malloc(sz);

	if(!ptr && sz)
		throw pd::exception_sys_t(pd::log::error, ENOMEM, "malloc: %m");

	return ptr;
}

void operator delete[](void *ptr) throw() { ::free(ptr); }

#pragma GCC visibility pop

#else

#include "out_fd.H"
#include "trace.H"
#include "list.H"

namespace pd { namespace {

class alloc_t {
	struct item_base_t {
		bool array;
		size_t size;
		trace_t<16> trace;
		spinlock_guard_t guard;

		inline item_base_t(bool _array, size_t _size, spinlock_t &_spinlock) throw() :
			array(_array), size(_size), trace(), guard(_spinlock) { }

		inline ~item_base_t() throw() { }
	};

	class item_t : public item_base_t, public list_item_t<item_t> {
	public:
		inline item_t(
			bool _array, size_t _size, item_t *&list, spinlock_t &_spinlock
		) throw() :
			item_base_t(_array, _size, _spinlock),
			list_item_t<item_t>(this, list) { guard.relax(); }

		inline ~item_t() throw() { guard.wakeup(); }

		inline bool check(bool _array) { return array == _array; }

		inline void print(out_t &out) {
			if(!this)
				return;

			out.print((void const *)(this + 1))(',')(' ').print(size).lf();

			trace.print(out);

			next->print(out);
		}

		inline void *operator new(size_t size, size_t body_size) {
			void *ptr = ::malloc(size + body_size);

			if(!ptr)
				throw exception_sys_t(log::error, ENOMEM, "malloc: %m");

			return ptr;
		}

		inline void operator delete(void *ptr) { ::free(ptr); }
	};

	item_t *list;
	spinlock_t spinlock;

public:
	inline alloc_t() throw() : list(NULL), spinlock() { }

	inline void *new_item(bool array, size_t size) {
		return new(size) item_t(array, size, list, spinlock) + 1;
	}

	inline void delete_item(bool array, void *ptr) {
		item_t *item = (item_t *)ptr - 1;
		assert(item->check(array));
		delete item;
	}

	inline ~alloc_t() throw() {
		if(!list)
			return;

		try {
			char obuf[1024];
			out_fd_t out(obuf, sizeof(obuf), 2);

			out(CSTR("Lost memory:")).lf();

			{
				spinlock_guard_t _guard(spinlock);
				list->print(out);
			}

			out.flush_all();
		}
		catch(...) { }
	}
};

static __init_priority(101) alloc_t alloc;

}} // namespace pd::(anonymous)

#pragma GCC visibility push(default)

void *operator new(size_t sz) { return pd::alloc.new_item(false, sz); }

void operator delete(void *ptr) throw() { pd::alloc.delete_item(false, ptr); }

void *operator new[](size_t sz) { return pd::alloc.new_item(true, sz); }

void operator delete[](void *ptr) throw() { pd::alloc.delete_item(true, ptr); }

#pragma GCC visibility pop

#endif
