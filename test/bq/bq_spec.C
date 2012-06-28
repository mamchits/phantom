#include <pd/bq/bq_spec.H>
#include <pd/base/assert.H>

#include <pthread.h>

using namespace pd;

bq_spec_decl(int, sp1);
bq_spec_decl(int, sp2);

static void *job(void *) {
	int *&i1 = sp1;

	assert((void *)&i1 == &sp1_non_bq);

	int *&i2 = sp2;

	assert((void *)&i2 == &sp2_non_bq);

	assert(&i1 != &i2);

	return &i1;
}

extern "C" int main() {
	void *res1 = job(NULL);

	pthread_t pthread;
	int err = pthread_create(&pthread, NULL, &job, NULL);
	assert(err == 0);

	void *res2 = res1;

	pthread_join(pthread, &res2);
	assert(res2 != res1);
	return 0;
}
