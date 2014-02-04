#include <pd/base/cond.H>

namespace pd {

class signal_t {
	size_t count;
	cond_t cond;

public:
	inline signal_t() throw() : count(0), cond() { }

	inline void wait() throw() {
		cond_t::handler_t handler(cond);

		while(!count)
			handler.wait();

		--count;

		if(count)
			handler.send();
	}

	inline void send() throw() {
		cond_t::handler_t handler(cond);
		++count;
		handler.send();
	}

	inline ~signal_t() throw() { }

	signal_t(signal_t const &) = delete;
	signal_t &operator=(signal_t const &) = delete;
};

} // namespace pd
