#include <pd/bq/bq.H>
#include <pd/bq/bq_heap.H>
#include <pd/bq/bq_cond.H>
#include <pd/bq/bq_util.H>

#include <pd/base/thr.H>
#include <pd/base/string.H>
#include <pd/base/exception.H>
#include <pd/base/out_fd.H>

#include "thr_signal.I"

#include <exception>

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

bq_thr_t bq_thr1, bq_thr2;

static thr::signal_t signal;

static bq_cont_t *cont;

char obuf[1024];
out_fd_t out(obuf, sizeof(obuf), 1);

struct a_t {
	inline a_t() { }
	inline ~a_t() {
		cont = bq_cont_current;

		if(std::uncaught_exception())
			out(CSTR("Ok 0")).lf().flush_all();

		out(CSTR("+a "))(bq_thr_get()->name()).lf().flush_all();

		if(bq_cont_deactivate("", wait_ready) == (bq_err_t)355)
			out(CSTR("Ok 4")).lf().flush_all();

		out(CSTR("-a "))(bq_thr_get()->name()).lf().flush_all();
	}
};

struct b_t {
	inline b_t() {
		if(!std::uncaught_exception())
			out(CSTR("Ok 1")).lf().flush_all();
	}

	inline ~b_t() {
		if(std::uncaught_exception())
			out(CSTR("Ok 2")).lf().flush_all();

		out(CSTR("+b "))(bq_thr_get()->name()).lf().flush_all();

		interval_t t = interval_zero;

		if(bq_thr2.switch_to(t, true) == bq_ok)
			out(CSTR("Ok 3")).lf().flush_all();

		out(CSTR("-b "))(bq_thr_get()->name()).lf().flush_all();

		bq_cont_set_msg(cont, (bq_err_t)355);
		bq_cont_activate(cont);
	}
};

class signal_t {
	bq_cond_t cond;
	size_t count;

public:
	inline signal_t(size_t _count) throw() : cond(), count(_count) { }

	inline ~signal_t() throw() { }

	inline void wait() {
		bq_cond_guard_t guard(cond);

		while(count)
			if(!bq_success(cond.wait(NULL)))
				throw exception_sys_t(log::error, errno, "cond.wait: %m");
	}

	inline void send() {
		bq_cond_guard_t guard(cond);

		if(!--count)
			cond.send();
	}
};


signal_t bq_signal(2);

static void job1(void *) {
	try {
		a_t a;
		throw 1;
	}
	catch(int i) {
		if(i == 1)
			out(CSTR("Ok 6")).lf().flush_all();
	}
	bq_signal.send();
}

static void job2(void *) {
	try {
		b_t b;
		throw 2;
	}
	catch(int i) {
		if(i == 2)
			out(CSTR("Ok 7")).lf().flush_all();
	}
	bq_signal.send();
}

static void job(void *) {
	bq_cont_create(&bq_thr1, &job1, NULL);
	bq_cont_create(&bq_thr1, &job2, NULL);
	bq_signal.wait();

	signal.send();
};

bq_cont_count_t cont_count(3);

extern "C" int main() {
	bq_thr1.init(STRING("thr1"), 1, interval_millisecond, cont_count, activate);
	bq_thr2.init(STRING("thr2"), 1, interval_millisecond, cont_count, activate);

	bq_cont_create(&bq_thr1, &job, NULL);
	signal.wait();

	bq_thr_t::stop();

	bq_thr2.fini();
	bq_thr1.fini();
}
