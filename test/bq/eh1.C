#include <pd/bq/bq_thr.H>
#include <pd/bq/bq_heap.H>

#include <pd/base/string.H>
#include <pd/base/time.H>
#include <pd/base/out_fd.H>

#include <exception>
#include <cxxabi.h>

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

static bq_cont_t *cont;

char obuf[1024];
out_fd_t out(obuf, sizeof(obuf), 1);

struct a_t {
	inline a_t() { }
	inline ~a_t() {
		cont = bq_cont_current;

		if(std::uncaught_exception())
			out(CSTR("Ok 0")).lf().flush_all();

		if(bq_cont_deactivate("", wait_ready) == (bq_err_t)388)
			out(CSTR("Ok 3")).lf().flush_all();
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

		bq_cont_set_msg(cont, (bq_err_t)388);
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

	bq_thr.init(STRING("thr"), 1, interval_millisecond, cont_count, activate);

	bq_cont_create(&bq_thr, &job1, NULL);
	bq_cont_create(&bq_thr, &job2, NULL);

	bq_thr_t::stop();

	bq_thr.fini();
}
