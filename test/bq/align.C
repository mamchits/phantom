/*
 * compile it with '-O2' to see the effect
 */

#ifndef __OPTIMIZE__
#error not interested without otimisation
#endif

#include <pd/bq/bq_thr.H>
#include <pd/bq/bq_heap.H>
#include <pd/bq/bq_spec.H>

#include <pd/base/string.H>

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


bq_spec_decl(int, sp1);
bq_spec_decl(int, sp2);

char md[16];

void job(void *) {
	int k;
	char local_md[16];

	for (k = 0; k < 16; k++) {
		md[k] ^= local_md[k];
	}
}

bq_cont_count_t cont_count(3);

extern "C" int main() {
	bq_thr_t bq_thr;

	bq_thr.init(STRING("thr"), 10, interval_second, cont_count, activate);

	bq_cont_create(&bq_thr, &job, NULL);

	bq_thr_t::stop();

	bq_thr.fini();

	return 0;
}
