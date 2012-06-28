// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "exception.H"

#include <malloc.h>

#ifndef DEBUG_ALLOC

#pragma GCC visibility push(default)

void *operator new(size_t sz) {
	void *ptr = ::malloc(sz);

	if(!ptr)
		throw pd::exception_sys_t(pd::log::error, ENOMEM, "malloc: %m");

	return ptr;
}

void operator delete(void *ptr) throw() { ::free(ptr); }

void *operator new[](size_t sz) {
	void *ptr = ::malloc(sz);

	if(!ptr)
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
	class item_t : public list_atomic_item_t<item_t> {
		size_t size;
		trace_t<16> trace;

	public:
		inline item_t(
			size_t _size, item_t *&list, thr::spinlock_t &_spin
		) throw() :
			list_atomic_item_t<item_t>(_spin),
			size(_size), trace() { link(this, list); }

		inline ~item_t() throw() { unlink(); }

		inline void print(out_t &out) throw() {
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
	thr::spinlock_t spin;

public:
	inline alloc_t() throw() : list(NULL), spin() { }

	inline void *new_item(size_t size) {
		return new(size) item_t(size, list, spin) + 1;
	}

	inline void delete_item(void *ptr) {
		delete ((item_t *)ptr - 1);
	}

	inline ~alloc_t() throw() {
		if(!list)
			return;

		char obuf[1024];
		out_fd_t out(obuf, sizeof(obuf), 2);

		out(CSTR("Lost memory:")).lf();

		{
			thr::spinlock_guard_t _guard(spin);
			list->print(out);
		}

		out.flush_all();
	}
};

static __init_priority(101) alloc_t alloc;

}} // namespace pd::(anonymous)

#pragma GCC visibility push(default)

void *operator new(size_t sz) { return pd::alloc.new_item(sz); }

void operator delete(void *ptr) throw() { pd::alloc.delete_item(ptr); }

void *operator new[](size_t sz) { return pd::alloc.new_item(sz); }

void operator delete[](void *ptr) throw() { pd::alloc.delete_item(ptr); }

#pragma GCC visibility pop

#endif
