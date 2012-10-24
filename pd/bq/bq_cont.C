// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_cont.H"
#include "bq_spec.H"
#include "bq_thr.H"

#include <pd/base/exception.H>
#include <pd/base/trace.H>
#include <pd/base/time.H>

#include <sys/mman.h>
#include <stdlib.h>

namespace pd {

#if defined(__i386__)

#define PAGE_SIZE (4096)
#define STACK_SIZE (2*1024*1024)
#define MISC_REG_NUM (3)

#elif defined(__x86_64__)

#define PAGE_SIZE (4096)
#define STACK_SIZE (8*1024*1024)
#define MISC_REG_NUM (5)

#elif defined(__arm__)

#define PAGE_SIZE (4096)
#define STACK_SIZE (2*1024*1024)
#define MISC_REG_NUM (8)

#else
#error Not implemented yet
#endif

class bq_spec_num_t {
	thr::spinlock_t spinlock;
	unsigned int value;

public:
	inline bq_spec_num_t() throw() : spinlock(), value(0) { }
	inline ~bq_spec_num_t() throw() { }

	inline unsigned int operator++() throw() {
		thr::spinlock_guard_t guard(spinlock);
		return ++value;
	}

	inline unsigned int operator++(int) throw() {
		thr::spinlock_guard_t guard(spinlock);
		return value++;
	}

	inline operator size_t() throw() {
		thr::spinlock_guard_t guard(spinlock);
		return value;
	}
};

static bq_spec_num_t __init_priority(101) bq_spec_num;

unsigned int bq_spec_reserve() throw() {
	return bq_spec_num++;
}

#if defined(__GXX_ABI_VERSION) && __GXX_ABI_VERSION == 1002

struct __cxa_eh_globals {
	void *caughtExceptions;
	unsigned int uncaughtExceptions;

#ifdef __ARM_EABI__
	void *propagatingExceptions;
#endif

	inline __cxa_eh_globals() throw() :
		caughtExceptions(NULL), uncaughtExceptions(0) { }
};

extern "C" __cxa_eh_globals *__cxa_get_globals() throw();

#else
// Check libstdc++-v3/libsupc++/unwind-cxx.h
#error Not Implemented yet
#endif

class state_t {
	void **stack_ptr; void **frame_ptr; void *pc;
	unsigned long misc[MISC_REG_NUM];

public:
	unsigned long save() throw();
	void __noreturn restore(int unsigned long) throw();

	inline void **stack() const throw() {
		return stack_ptr;
	}

	inline state_t() throw() { }
	inline ~state_t() throw() { }

	friend class bq_cont_t;
};

__thread bq_cont_t *bq_cont_current = NULL;

void __noreturn bq_cont_run(void (*fun)(void *), void *arg, void *stack);

class bq_cont_t {
	thr::spinlock_t spinlock;
	bq_thr_t *bq_thr;
	bq_err_t msg;
	state_t state, parent_state;
	__cxa_eh_globals eh_globals;
	char const *where_str;

	bq_cont_state_t cont_state;
	timeval_t stat_timeval;
	interval_t stat[bq_cont_states];

	size_t const spec_num;

	class guard_t {
		bq_cont_t *cont;
		bq_cont_t *parent;

	public:
		inline guard_t(bq_cont_t *_cont) throw() :
			cont(_cont), parent(bq_cont_current) {

			cont->spinlock.lock();

			bq_cont_current = cont;

			bq_cont_stat_update(parent, wait_another);

			cont->where_str = "running";
		}

		inline ~guard_t() throw() {
			bq_cont_stat_update(parent, running);

			bq_cont_current = parent;
			cont->spinlock.unlock();
		}
	};

	friend class guard_t;

public:
	static size_t count;
	static thr::spinlock_t count_spinlock;

	void *operator new(size_t size);
	void operator delete(void *ptr);

	inline bq_cont_t(bq_thr_t *_bq_thr) throw() :
		spinlock(), bq_thr(_bq_thr), msg(bq_ok), where_str(""),
		cont_state(running), stat_timeval(timeval_current()),
		spec_num(bq_spec_num) {

		for(unsigned int i = 0; i < spec_num; ++i)
			((void **)this)[-2 - (int)i] = NULL; // (*this)[i] = NULL;

		for(unsigned int i = 0; i < bq_cont_states; ++i)
			stat[i] = interval_zero;

		thr::spinlock_guard_t guard(count_spinlock);
		++count;
	}

	inline ~bq_cont_t() throw() {
		bq_thr->cont_count().dec();

		thr::spinlock_guard_t guard(count_spinlock);
		--count;
	}

	inline void stat_print() {
		log_debug(
			"cont stats: %lu %lu %lu %lu %lu",
			(unsigned long)(stat[running] / interval_millisecond),
			(unsigned long)(stat[wait_another] / interval_millisecond),
			(unsigned long)(stat[wait_event] / interval_millisecond),
			(unsigned long)(stat[wait_ready] / interval_millisecond),
			(unsigned long)(stat[wait_ext] / interval_millisecond)
		);
	}

	inline void eh_globals_swap() {
		__cxa_eh_globals &eh_globals_sys = *__cxa_get_globals();
		__cxa_eh_globals tmp = eh_globals_sys;
		eh_globals_sys = eh_globals;
		eh_globals = tmp;
	}

	inline void stat_update(bq_cont_state_t new_state) {
		timeval_t stat_timeval_new = timeval_current();

		if(cont_state < bq_cont_states)
			stat[cont_state] += (stat_timeval_new - stat_timeval);

		cont_state = new_state;
		stat_timeval = stat_timeval_new;
	}

	inline bool run(void (*fun)(void *), void *arg) {
		unsigned long res;
		{
			guard_t guard(this);

			if(!(res = parent_state.save())) {
				eh_globals_swap();
				bq_cont_run(fun, arg, stack());
			}
		}
		return (res == 1);
	}

	inline void set_msg(bq_err_t _msg) throw() { msg = _msg; }

	inline bool activate() throw() {
		unsigned long res;
		{
			guard_t guard(this);

			if(!(res = parent_state.save())) {
				eh_globals_swap();
				state.restore(1);
			}
		}

		return (res == 1);
	}

	inline bq_err_t deactivate(char const *_where, bq_cont_state_t new_state) throw() {
		where_str = _where;

		stat_update(new_state);

		if(!state.save()) {
			eh_globals_swap();
			parent_state.restore(1);
		}

		stat_update(running);

		return msg;
	}

	inline bq_thr_t *bq_thr_get() const throw() { return bq_thr; }

	inline bool bq_thr_set(bq_thr_t *_bq_thr) throw() {
		bq_cont_count_t &old_count = bq_thr->cont_count();
		bq_cont_count_t &new_count = _bq_thr->cont_count();

		if(&old_count != &new_count) {
			if(!new_count.inc())
				return false;

			old_count.dec();
		}

		bq_thr = _bq_thr;
		return true;
	}

	inline void __noreturn fini() throw() {
		stat_update(none);
		//stat_print();

		eh_globals_swap();
		parent_state.restore(2);
	}

	inline char *stack() const throw() {
		return (char *)((((uintptr_t)this) - (spec_num + 2) * sizeof(void *)) & ~(uintptr_t)15);
	}

	inline size_t stack_size() const throw() {
		return ((char *)this - (char *)state.stack());
	}

	inline void *&operator[](unsigned int i) {
		if(this && i < spec_num)
			return ((void **)this)[-2 - (int)i];

		throw exception_sys_t(log::error, EINVAL, "bq_spec: %m");
	}

	inline char const *where() const throw() { return where_str; }
};

void *&bq_spec(unsigned int id, void *&non_bq) {
	return bq_cont_current ? (*bq_cont_current)[id] : non_bq;
}

void __noreturn bq_cont_proc(void (*fun)(void *), void *arg) {
	bq_cont_t *cont = bq_cont_current;
	interval_t prio = interval_zero;

	cont->bq_thr_get()->switch_to(prio);

	(*fun)(arg);

	cont->fini();
}

size_t bq_cont_t::count = 0;
thr::spinlock_t bq_cont_t::count_spinlock;

bq_err_t bq_cont_create(bq_thr_t *bq_thr, void (*fun)(void *), void *arg) {
	if(!bq_thr->cont_count().inc())
		return bq_overload;

	bq_cont_t *cont = new bq_cont_t(bq_thr);

	if(!cont->run(fun, arg)) delete cont;

	return bq_ok;
}

size_t bq_cont_count() throw() {
	thr::spinlock_guard_t guard(bq_cont_t::count_spinlock);
	return bq_cont_t::count;
}

void bq_cont_set_msg(bq_cont_t *cont, bq_err_t msg) throw() {
	if(cont)
		cont->set_msg(msg);
}

void bq_cont_activate(bq_cont_t *cont) throw() {
	if(cont && !cont->activate()) delete cont;
}

bq_err_t bq_cont_deactivate(char const *where, bq_cont_state_t state) throw() {
	bq_cont_t *cont = bq_cont_current;

	return cont ? cont->deactivate(where, state) : bq_illegal_call;
}

void bq_cont_stat_update(bq_cont_t *cont, bq_cont_state_t new_state) throw() {
	if(cont)
		cont->stat_update(new_state);
}

bool bq_thr_set(bq_thr_t *bq_thr) throw() {
	bq_cont_t *cont = bq_cont_current;

	return cont ? cont->bq_thr_set(bq_thr) : true;
}

bq_thr_t *bq_thr_get() throw() {
	bq_cont_t *cont = bq_cont_current;

	return cont ? cont->bq_thr_get() : NULL;
}

void const *bq_cont_id(bq_cont_t const *cont) throw() {
	return cont;
}

char const *bq_cont_where(bq_cont_t const *cont) throw() {
	return cont ? cont->where() : "";
}

size_t bq_cont_stack_size(bq_cont_t const *cont) throw() {
	return cont ? cont->stack_size() : 0;
}

class bq_stack_pool_t {
	thr::spinlock_t spinlock;
	size_t num;
	size_t wnum;

	struct item_t {
		item_t *next;

		inline char *stack() throw() {
			return ((char *)this) + sizeof(*this) - STACK_SIZE;
		}

		static inline item_t *obj(char *stack) throw() {
			return (item_t *)(stack + STACK_SIZE - sizeof(item_t));
		}

	} *pool;

public:
	inline bq_stack_pool_t() throw() :
		spinlock(), num(0), wnum(0), pool(NULL) { }

	inline char *alloc() {
		{
			thr::spinlock_guard_t guard(spinlock);
			++wnum;

			if(num) {
				item_t *item = pool;
				pool = item->next;
				--num;

				return item->stack();
			}
		}

		char *stack = (char *)mmap(
			NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0
		);

		if(stack == MAP_FAILED)
			throw exception_sys_t(log::error, errno, "mmap: %m");

		if (mprotect(stack, PAGE_SIZE, 0) < 0)
			throw exception_sys_t(log::error, errno, "mprotect: %m");

		return stack;
	}

	inline void free(char *stack) throw() {
		{
			thr::spinlock_guard_t guard(spinlock);
			if(num <= wnum--) {
				item_t *item = item_t::obj(stack);

				item->next = pool;
				pool = item;
				++num;
				return;
			}
		}

		munmap(stack, STACK_SIZE);
	}

	inline ~bq_stack_pool_t() throw() {
		while(pool) {
			item_t *item = pool;
			pool = item->next;
			munmap(item->stack(), STACK_SIZE);
		}
	}

	inline bq_stack_pool_info_t get_info() throw() {
		bq_stack_pool_info_t info;
		{
			thr::spinlock_guard_t guard(spinlock);
			info.wsize = wnum;
			info.size = num;
		}
		return info;
	}
};

static bq_stack_pool_t bq_stack_pool;

void *bq_cont_t::operator new(size_t size) {
	char *stack = bq_stack_pool.alloc();

	void *ptr = stack + STACK_SIZE - size;

	((char **)ptr)[-1] = stack;

	return ptr;
}

void bq_cont_t::operator delete(void *ptr) {
	bq_stack_pool.free(((char **)ptr)[-1]);
}

bq_stack_pool_info_t bq_stack_pool_get_info() throw() {
	return bq_stack_pool.get_info();
}

} // namespace pd
