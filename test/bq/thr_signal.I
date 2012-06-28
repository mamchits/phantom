#include <pd/base/thr.H>

namespace pd { namespace thr {

class signal_t : mutex_t {
	size_t count;
	pthread_cond_t pthread_cond;

public:
	inline signal_t() throw() : count(0) {
		pthread_cond_init(&pthread_cond, NULL);
	}

	inline void wait() throw() {
		mutex_guard_t guard(*this);

		while(!count)
			pthread_cond_wait(&pthread_cond, &pthread_mutex);

		--count;

		if(count)
			 pthread_cond_signal(&pthread_cond);
	}

	inline void send() throw() {
		mutex_guard_t guard(*this);
		++count;
		pthread_cond_signal(&pthread_cond);
	}

	inline ~signal_t() throw() {
		pthread_cond_destroy(&pthread_cond);
	}

private: // don't use
	signal_t(signal_t const &);
	signal_t &operator=(signal_t const &);
};

}} // namespace pd::thr
