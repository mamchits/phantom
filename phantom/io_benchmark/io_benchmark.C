// This file is part of the phantom::io_benchmark module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "method.H"
#include "../io.H"
#include "../module.H"

#include <pd/bq/bq_job.H>
#include <pd/bq/bq_cond.H>
#include <pd/bq/bq_util.H>

#include <pd/base/config_enum.H>
#include <pd/base/exception.H>
#include <pd/base/random.H>
#include <pd/base/ref.H>

#include <sys/signal.h>
#include <unistd.h>

namespace phantom {

MODULE(io_benchmark);

class io_benchmark_t : public io_t {
public:
	typedef io_benchmark::times_t times_t;
	typedef io_benchmark::stat_t stat_t;
	typedef io_benchmark::method_t method_t;

private:
	virtual void init();
	virtual void run();
	virtual void stat(out_t &out, bool clear);
	virtual void fini();

	unsigned int instances;
	method_t &method;
	interval_t random_sleep_max;
	stat_t *statp;

	class signal_t : public ref_count_atomic_t {
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

		friend class ref_t<signal_t>;
	};

	void instance_proc(ref_t<signal_t> signal);

public:
	struct config_t : io_t::config_t {
		sizeval_t instances;

		config_binding_type_ref(method_t);
		config::objptr_t<method_t> method;

		config_binding_type_ref(times_t);
		config::objptr_t<times_t> times;

		interval_t random_sleep_max;
		config::enum_t<bool> human_readable_report;

		inline config_t() throw() :
			instances(16), method(), times(), random_sleep_max(interval_zero),
			human_readable_report(true) { }

		inline void check(in_t::ptr_t const &ptr) const {
			io_t::config_t::check(ptr);

			if(instances >= sizeval_mega)
				config::error(ptr, "instances is too big");

			if(random_sleep_max < interval_zero)
				config::error(ptr, "random_sleep_max is negative");

			if(!method)
				config::error(ptr, "method is required");

			if(!times)
				config::error(ptr, "times is required");
		}
	};

	inline io_benchmark_t(string_t const &name, config_t const &config) :
		io_t(name, config),
		instances(config.instances), method(*config.method),
		random_sleep_max(config.random_sleep_max) {

		statp = new stat_t(method, *config.times, config.human_readable_report);
	}

	inline ~io_benchmark_t() throw() { delete statp; }
};

namespace io_benchmark {
config_binding_sname(io_benchmark_t);
config_binding_value(io_benchmark_t, instances);
config_binding_type(io_benchmark_t, method_t);
config_binding_value(io_benchmark_t, method);
config_binding_type(io_benchmark_t, times_t);
config_binding_value(io_benchmark_t, times);
config_binding_value(io_benchmark_t, random_sleep_max);
config_binding_value(io_benchmark_t, human_readable_report);
config_binding_parent(io_benchmark_t, io_t, 1);
config_binding_ctor(io_t, io_benchmark_t);
}

void io_benchmark_t::init() { method.init(); }

void io_benchmark_t::fini() { method.fini(); }

void io_benchmark_t::instance_proc(ref_t<signal_t> signal) {
	class signal_guard_t {
		signal_t &signal;
	public:
		inline signal_guard_t(signal_t &_signal) : signal(_signal) { }
		inline ~signal_guard_t() { safe_run(signal, &signal_t::send); }
	} signal_guard(*signal);

	while(1) {
		if(random_sleep_max > interval_zero) {
			interval_t interval = random_F() * random_sleep_max;
			if(bq_sleep(&interval) < 0)
				throw exception_sys_t(log::error, errno, "bq_sleep: %m");
		}

		if(!method.test(*statp))
			break;
	}
}

void io_benchmark_t::run() {
	ref_t<signal_t> signal = new signal_t(instances);

	for(unsigned int i = 0; i < instances; ++i) {
		string_t _name = string_t::ctor_t(name.size() + 1 + 6 + 1)
			(name)('[').print(i)(']')
		;

		try {
			bq_job_t<typeof(&io_benchmark_t::instance_proc)>::create(
				_name, scheduler.bq_thr(), *this, &io_benchmark_t::instance_proc, signal
			);
		}
		catch(exception_t const &ex) {
			ex.log();
		}
	}

	signal->wait();

	kill(getpid(), SIGTERM);
}

void io_benchmark_t::stat(out_t &out, bool clear) {
	statp->print(out, clear);
}

} // namespace phantnom
