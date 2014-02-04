// This file is part of the phantom::io_benchmark module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "method.H"

#include <pd/base/config.H>
#include <pd/base/config_list.H>
#include <pd/base/size.H>

namespace phantom { namespace io_benchmark {

class times_simple_t : public times_t {
public:
	struct config_t {
		interval_t max, min;
		sizeval_t steps;

		inline config_t() :
			max(interval::second), min(interval::millisecond), steps(30) { }

		inline void check(in_t::ptr_t const &ptr) const {
			if(min <= interval::zero)
				config::error(ptr, "min must be a positive interval");

			if(max <= min)
				config::error(ptr, "max must be greater than min");

			if(!steps)
				config::error(ptr, "steps must be a positive number");

			if(steps >= sizeval::kilo)
				config::error(ptr, "steps is too big");
		}
	};

	struct ctor_t : times_t::ctor_t {
		config_t const &config;

		inline ctor_t(config_t const &_config) throw() : config(_config) { }

		inline ~ctor_t() throw() { }

		virtual size_t size() const throw() { return config.steps + 1; }

		virtual void fill(interval_t *_steps) const throw() {
			size_t n = config.steps;

			double l_max = __builtin_log((config.max / interval::microsecond) + 0.);
			double l_min = __builtin_log((config.min / interval::microsecond) + 0.);

			for(size_t i = 0; i <= n; ++i)
				_steps[i] = interval::microsecond * __builtin_llrint(
					__builtin_exp((i * l_min + (n - i) * l_max) / n)
				);
		}
	};

	inline times_simple_t(string_t const &name, config_t const &config) :
		times_t(name, ctor_t(config)) { }

	inline ~times_simple_t() throw() { }
};

namespace times_simple {
config_binding_sname(times_simple_t);
config_binding_value(times_simple_t, max);
config_binding_value(times_simple_t, min);
config_binding_value(times_simple_t, steps);
config_binding_cast(times_simple_t, times_t);
config_binding_ctor(times_t, times_simple_t);
}

class times_list_t : public times_t {
public:
	struct config_t {
		config::list_t<interval_t> values;

		inline config_t() { }

		inline void check(in_t::ptr_t const &ptr) const {
			interval_t last = interval::zero;

			for(typeof(values._ptr()) _ptr = values; _ptr; ++_ptr) {
				interval_t t = _ptr.val();

				if(t <= last)
					config::error(ptr, "values is not monotonic nonzero list");

				last = t;
			}

			if(last == interval::zero)
				config::error(ptr, "values is empty");
		}
	};

	struct ctor_t : times_t::ctor_t {
		config_t const &config;
		size_t list_size;

		inline ctor_t(config_t const &_config) throw() :
			config(_config), list_size() {

			for(typeof(config.values._ptr()) ptr = config.values; ptr; ++ptr)
				++list_size;
		}

		inline ~ctor_t() throw() { }

		virtual size_t size() const throw() { return list_size; }

		virtual void fill(interval_t *_steps) const throw() {
			size_t i = list_size;

			for(typeof(config.values._ptr()) ptr = config.values; ptr; ++ptr)
				_steps[--i] = ptr.val();
		}
	};

	inline times_list_t(string_t const &name, config_t const &config) :
		times_t(name, ctor_t(config)) { }

	inline ~times_list_t() throw() { }
};

namespace times_list {
config_binding_sname(times_list_t);
config_binding_value(times_list_t, values);
config_binding_cast(times_list_t, times_t);
config_binding_ctor(times_t, times_list_t);
}

}} // namespace phantom::io_benchmark
