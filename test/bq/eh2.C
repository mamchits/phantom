#include <pd/bq/bq_heap.H>
#include <pd/bq/bq_cond.H>
#include <pd/bq/bq_util.H>

#include <pd/base/string.H>
#include <pd/base/exception.H>
#include <pd/base/out_fd.H>

#include "thr_signal.I"
#include "thr_name.I"

#include <exception>

using namespace pd;

bq_thr_t bq_thr1, bq_thr2;

static signal_t signal;

static bq_cont_t *cont;

char obuf[1024];
out_fd_t out(obuf, sizeof(obuf), 1);

struct a_t {
	inline a_t() { }
	inline ~a_t() {
		cont = bq_cont_current;

		if(std::uncaught_exception())
			out(CSTR("Ok 0")).lf().flush_all();

		out(CSTR("+a "))(thr_name()).lf().flush_all();

		bq_cont_deactivate("");

		out(CSTR("Ok 4")).lf().flush_all();

		out(CSTR("-a "))(thr_name()).lf().flush_all();
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

		out(CSTR("+b "))(thr_name()).lf().flush_all();

		interval_t t = interval::zero;

		if(bq_thr2.switch_to(t, true) == bq_ok)
			out(CSTR("Ok 3")).lf().flush_all();

		out(CSTR("-b "))(thr_name()).lf().flush_all();

		bq_cont_activate(cont);
	}
};

class bq_signal_t {
	bq_cond_t cond;
	size_t count;

public:
	inline bq_signal_t(size_t _count) throw() : cond(), count(_count) { }

	inline ~bq_signal_t() throw() { }

	inline void wait() {
		bq_cond_t::handler_t handler(cond);

		while(count)
			handler.wait();
	}

	inline void send() {
		bq_cond_t::handler_t handler(cond);

		if(!--count)
			handler.send();
	}
};


bq_signal_t bq_signal(2);

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
	bq_thr1.init(1, interval::millisecond, cont_count, STRING("thr1"));
	bq_thr2.init(1, interval::millisecond, cont_count, STRING("thr2"));

	bq_cont_create(&bq_thr1, &job, NULL);
	signal.wait();

	bq_thr_t::stop();

	bq_thr2.fini();
	bq_thr1.fini();
}
