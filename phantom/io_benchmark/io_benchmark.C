// This file is part of the phantom::io_benchmark module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "method.H"
#include "../io.H"
#include "../module.H"
#include "../scheduler.H"

#include <pd/bq/bq_job.H>
#include <pd/bq/bq_cond.H>
#include <pd/bq/bq_util.H>

#include <pd/base/config_enum.H>
#include <pd/base/exception.H>
#include <pd/base/ref.H>

namespace phantom {

MODULE(io_benchmark);

class io_benchmark_t : public io_t {
public:
	typedef io_benchmark::times_t times_t;
	typedef io_benchmark::method_t method_t;

private:
	virtual void init() {
		times.init(name);
		method.init(name);
	}

	virtual void stat_print() const {
		times.stat_print(name);
		method.stat_print(name);
	}

	virtual void fini() { method.fini(name); }

	virtual void run() const;

	unsigned int instances;
	times_t &times;
	method_t &method;

	class signal_t : public ref_count_atomic_t {
		bq_cond_t cond;
		size_t count;

	public:
		inline signal_t(size_t _count) throw() : cond(), count(_count) { }

		inline ~signal_t() throw() { }

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

		friend class ref_t<signal_t>;
	};

	void proc(ref_t<signal_t> signal) const;

public:
	struct config_t : io_t::config_t {
		sizeval_t instances;

		config_binding_type_ref(method_t);
		config::objptr_t<method_t> method;

		config_binding_type_ref(times_t);
		config::objptr_t<times_t> times;

		inline config_t() throw() :
			instances(16), method(), times() { }

		inline void check(in_t::ptr_t const &ptr) const {
			io_t::config_t::check(ptr);

			if(instances >= sizeval::mega)
				config::error(ptr, "instances is too big");

			if(!method)
				config::error(ptr, "method is required");

			if(!times)
				config::error(ptr, "times is required");
		}
	};

	inline io_benchmark_t(string_t const &name, config_t const &config) :
		io_t(name, config, /* active */ true),
		instances(config.instances),
		times(*config.times), method(*config.method) { }

	inline ~io_benchmark_t() throw() { }
};

namespace io_benchmark {
config_binding_sname(io_benchmark_t);
config_binding_value(io_benchmark_t, instances);
config_binding_type(io_benchmark_t, method_t);
config_binding_value(io_benchmark_t, method);
config_binding_type(io_benchmark_t, times_t);
config_binding_value(io_benchmark_t, times);
config_binding_removed(io_benchmark_t, random_sleep_max, "random_sleep_max is obsoleted");
config_binding_removed(io_benchmark_t, human_readable_report, "human_readable_report is obsoleted");
config_binding_parent(io_benchmark_t, io_t);
config_binding_ctor(io_t, io_benchmark_t);
}

void io_benchmark_t::proc(ref_t<signal_t> signal) const {
	class signal_guard_t {
		signal_t &signal;
	public:
		inline signal_guard_t(signal_t &_signal) : signal(_signal) { }
		inline ~signal_guard_t() { safe_run(signal, &signal_t::send); }
	} signal_guard(*signal);

	while(1) {
		if(!method.test(times))
			break;
	}
}

void io_benchmark_t::run() const {
	method.run(name);

	{
		ref_t<signal_t> signal = new signal_t(instances);
		char const *fmt = log::number_fmt(instances);

		auto job = bq_job(&io_benchmark_t::proc)(*this, signal);

		for(unsigned int i = 0; i < instances; ++i) {
			try {
				string_t _name = string_t::ctor_t(8).print(i, fmt);
				log::handler_t handler(_name);

				job->run(scheduler.bq_thr());
			}
			catch(exception_t const &) { }
		}

		signal->wait();
	}
}

} // namespace phantnom
