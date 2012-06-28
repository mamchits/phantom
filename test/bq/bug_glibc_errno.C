/*
 * compile it with '-O2' to see the effect
 */

#ifndef __OPTIMIZE__
#error not interested without otimisation
#endif

#define __bq_errno_workaround_disabled
#include <errno.h>
#undef __bq_errno_workaround_disabled

#define __bq_pthread_self_workaround_disabled
#include <pthread.h>
#undef __bq_pthread_self_workaround_disabled

#include <pd/bq/bq_thr.H>
#include <pd/bq/bq_heap.H>

#include <pd/base/thr.H>
#include <pd/base/string.H>
#include <pd/base/out_fd.H>

#include "thr_signal.I"

using namespace pd;

class activate_t : public bq_cont_activate_t {
	virtual void operator()(bq_heap_t::item_t *item, bq_err_t err) {
		bq_cont_set_msg(item->cont, err);
		bq_cont_activate(item->cont);
	}
public:
	inline activate_t() throw() { }
	inline ~activate_t() throw() { }
};

activate_t activate;

thr::signal_t signal;

bq_thr_t bq_thr1, bq_thr2;

char obuf[1024];
out_fd_t out(obuf, sizeof(obuf), 1);

static void job(void *) {
	int *errno_ptr1 = &errno;
	pthread_t pthread1 = pthread_self();
	string_t const &thr1_name = bq_thr_get()->name();

	interval_t t = interval_zero;
	if(bq_thr2.switch_to(t) != bq_ok)
		fatal("can't switch");

	int *errno_ptr2 = &errno;
	pthread_t pthread2 = pthread_self();
	string_t const &thr2_name = bq_thr_get()->name();

	if(errno_ptr1 == errno_ptr2) {
		out
			(CSTR("libc BUG! &errno@"))(thr1_name)
			(CSTR(" == &errno@"))(thr2_name).lf().flush_all()
		;
	}
	else {
#ifdef __bq_errno_workaround
		out(CSTR("&errno is ok (workaround activated)")).lf().flush_all();
#else
		out(CSTR("&errno is ok, it seems the bug fixed")).lf().flush_all();
#endif
	}

	if(pthread1 == pthread2) {
		out
			(CSTR("libpthread BUG! pthread_self()@"))(thr1_name)
			(CSTR(" == pthread_self()@"))(thr2_name).lf().flush_all()
		;
	}
	else {
#ifdef __bq_pthread_self_workaround
		out(CSTR("pthread_self() is ok (workaround activated)")).lf().flush_all();
#else
		out(CSTR("pthread_self() is ok, it seems the bug fixed")).lf().flush_all();
#endif
	}

	signal.send();
}

bq_cont_count_t cont_count(1);

extern "C" int main() {
	bq_thr1.init(STRING("thr1"), 10, interval_second, cont_count, activate);
	bq_thr2.init(STRING("thr2"), 10, interval_second, cont_count, activate);

	if(bq_cont_create(&bq_thr1, &job, NULL) != bq_ok)
		fatal("can't create cont");

	signal.wait();

	bq_thr_t::stop();

	bq_thr2.fini();
	bq_thr1.fini();
}
