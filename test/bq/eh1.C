#include <pd/bq/bq_thr.H>
#include <pd/bq/bq_heap.H>
#include <pd/bq/bq_cond.H>

#include <pd/base/string.H>
#include <pd/base/time.H>
#include <pd/base/out_fd.H>

#include <exception>
#include <cxxabi.h>

using namespace pd;

static bq_cont_t *cont;
bq_cond_t cond;

char obuf[1024];
out_fd_t out(obuf, sizeof(obuf), 1);

struct a_t {
	inline a_t() {
		bq_cond_t::handler_t handler(cond);
		cont = bq_cont_current;
		handler.send();
	}
	inline ~a_t() {
		if(std::uncaught_exception())
			out(CSTR("Ok 0")).lf().flush_all();

		bq_cont_deactivate("");

		out(CSTR("Ok 3")).lf().flush_all();
	}
};

struct b_t {
	inline b_t() {
		{
			bq_cond_t::handler_t handler(cond);
			while(!cont)
				try { handler.wait(); } catch(...) { }
		}

		if(!std::uncaught_exception())
			out(CSTR("Ok 1")).lf().flush_all();
	}

	inline ~b_t() {
		if(std::uncaught_exception())
			out(CSTR("Ok 2")).lf().flush_all();

		bq_cont_activate(cont);
	}
};

static void job1(void *) {
	try {
		a_t a;
		throw 1;
	}
	catch(int i) {
		if(i == 1)
			out(CSTR("Ok 4")).lf().flush_all();
	}
}

static void job2(void *) {
	try {
		b_t b;
		throw 2;
	}
	catch(int i) {
		if(i == 2)
			out(CSTR("Ok 5")).lf().flush_all();
	}
}

bq_cont_count_t cont_count(3);

extern "C" int main() {
	bq_thr_t bq_thr;

	bq_thr.init(1, interval::millisecond, cont_count, STRING("thr"));

	bq_cont_create(&bq_thr, &job1, NULL);
	bq_cont_create(&bq_thr, &job2, NULL);

	bq_thr_t::stop();

	bq_thr.fini();
}
