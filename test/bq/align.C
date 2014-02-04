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

bq_spec_decl(int, sp1);
bq_spec_decl(int, sp2);

char md[16];

#pragma GCC diagnostic ignored "-Wuninitialized"

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

	bq_thr.init(10, interval::second, cont_count, STRING("thr"));

	bq_cont_create(&bq_thr, &job, NULL);

	bq_thr_t::stop();

	bq_thr.fini();

	return 0;
}
