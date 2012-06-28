// This file is part of the phantom::io_benchmark module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "method.H"

#include <pd/base/config.H>
#include <pd/base/config_list.H>
#include <pd/base/frac.H>
#include <pd/base/exception.H>

namespace pd {

class percent_t {
	unsigned long v;

public:
	template<class x_t>
	inline percent_t(x_t val, x_t rel) throw() {
		if(((val * 10000) / 10000) != val) {
			val /= 10000;
			rel /= 10000;
		}

		v = (val * 10000) / rel;
	}

	inline unsigned long int_part() const throw() {
		return v / 100;
	}

	inline unsigned long frac_part() const throw() {
		return v % 100;
	}

	inline ~percent_t() throw() { }
};

template<>
void out_t::helper_t<percent_t>::print(
	out_t &out, percent_t const &p, char const *
) {
	out
		.print(p.int_part(), "3")('.')
		.print(p.frac_part(), "02")('%')
	;
}

} // namespace pd

namespace phantom { namespace io_benchmark {

class stat_t::res_t {
	class item_t {
		unsigned long count_sum;
		size_t count_num;
		unsigned long *counts;
		descr_t const *descr;

	public:
		inline item_t() throw() :
			count_sum(0), count_num(0), counts(NULL), descr(NULL) { }

		inline ~item_t() throw() { if(counts) delete [] counts; }

		inline void setup(descr_t const *_descr) {
			descr = _descr;
			count_num = descr->value_max();
			counts = new unsigned long[count_num + 1]; // last is "unknown"
			for(size_t i = 0; i <= count_num; ++i) counts[i] = 0;
		}

		inline void update(size_t value) {
			++count_sum;
			++counts[value < count_num ? value : count_num];
		}

		inline item_t &operator+=(item_t const &item) {
			assert(descr == item.descr);

			count_sum += item.count_sum;
			for(size_t i = 0; i <= count_num; ++i) counts[i] += item.counts[i];

			return *this;
		}

		inline void print(out_t &out, char const *fmt, bool hrr_flag) {
			if (hrr_flag) {
				out.lf()('=')('=')(' ');
				descr->print_header(out);
				out.lf();

				for(size_t v = 0; v <= count_num; ++v) {
					if(counts[v]) {
						out
							.print(counts[v], fmt)(' ')
							.print(percent_t(counts[v], count_sum))(':')(' ')
						;

						descr->print_value(out, v);

						out.lf();
					}
				}
				out('-')('-').lf();
				out.print(count_sum, fmt)(CSTR("        : total")).lf();
			}
			else {
				descr->print_header(out);
				for(size_t v = 0; v <= count_num; ++v) {
					if(counts[v])
						out('\t').print(v)(':').print(counts[v]);
				}
				out.lf();
			}
		}

		friend class res_t;
	};

	size_t size;
	item_t *items;

	timeval_t time_start;
	sizeval_t tests_all;
	interval_t time_real, time_events;

	sizeval_t tests_sucess;
	sizeval_t size_in, size_out;

	sizeval_t events;

public:
	inline res_t(size_t maxi, descr_t const *const *descr) :
		time_start(timeval_current()),
		tests_all(0), time_real(0), time_events(0),
		tests_sucess(0), size_in(0), size_out(0), events(0) {

		items = new item_t[size = maxi];
		try {
			for(size_t i = 0; i < size; ++i)
				items[i].setup(descr[i]);
		}
		catch(...) {
			delete [] items;
			throw;
		}
	}

	inline res_t(res_t const &res, bool) :
		time_start(timeval_current()),
		tests_all(0), time_real(0), time_events(0),
		tests_sucess(0), size_in(0), size_out(0), events(0) {

		items = new item_t[size = res.size];

		try {
			for(size_t i = 0; i < size; ++i)
				items[i].setup(res.items[i].descr);
		}
		catch(...) {
			delete [] items;
			throw;
		}
	}

	inline ~res_t() throw() { if(items) delete [] items; }

	void update(size_t index, size_t value) {
		if(index < size)
			items[index].update(value);
	}

	inline void update_time(
		interval_t _time_real, interval_t _time_events
	) {
		++tests_all;
		time_real += _time_real;
		time_events += _time_events;
	}

	inline void update_size(sizeval_t _size_in, sizeval_t _size_out) {
		++tests_sucess;
		size_in += _size_in;
		size_out += _size_out;
	}

	inline void event() {
		++events;
	}

	inline res_t &operator+=(res_t const &res) {
		for(size_t i = 0; i < size; ++i)
			items[i] += res.items[i];

		tests_all += res.tests_all;
		time_real += res.time_real;
		time_events += res.time_events;

		tests_sucess += res.tests_sucess;
		size_in += res.size_in;
		size_out += res.size_out;

		events += res.events;

		return *this;
	}

	void print(out_t &out, bool hrr_flag) const;

private: // don't use
	res_t(res_t const &res);
	res_t &operator=(res_t const &res);
};

void stat_t::res_t::print(out_t &out, bool hrr_flag) const {
	interval_t time_total = timeval_current() - time_start;
	if(time_total == interval_zero) time_total = interval_microsecond;
	interval_t _time_real = time_real > interval_zero ? time_real : interval_microsecond;

	char fmt[2];
	{
		int k = 1;
		size_t c = tests_all;
		while(c /= 10) ++k;
		if(k > 9) k = 9;
		fmt[0] = '0' + k;
		fmt[1] = '\0';
	}

	for(size_t i = 0; i < size; ++i)
		items[i].print(out, fmt, hrr_flag);

	frac_t<interval_t> rate_frac =
		frac_t<interval_t>(interval_second, time_total);

	if (hrr_flag) {
		out.lf()(CSTR("== overall")).lf();

		out
			(CSTR("time: ")).print(time_total)(CSTR(" ("))
			.print(tests_sucess * rate_frac)(CSTR("rps), self-loading: "))
			.print(percent_t(_time_real - time_events, _time_real)).lf()
		;

		out
			(CSTR("out: ")).print(size_out, ".9")(CSTR(" ("))
			.print(size_out * rate_frac, ".9")
			(CSTR("bps), in: ")).print(size_in, ".9")(CSTR(" ("))
			.print(size_in * rate_frac, ".9")
			(CSTR("bps), ev: ")).print(events, ".9")(CSTR(" ("))
			.print(events * rate_frac, ".9")
			(CSTR("eps)")).lf()
		;
	}
	else {
		out
			(CSTR("overall\t"))
			.print(time_total / interval_millisecond)('\t')
			.print(tests_sucess * rate_frac)('\t')
			.print(percent_t(_time_real - time_events, _time_real))('\t')
			.print((uint64_t)size_out)('\t')
			.print((uint64_t)(size_out * rate_frac))('\t')
			.print((uint64_t)size_in)('\t')
			.print((uint64_t)(size_in * rate_frac))('\t')
			.print((uint64_t)events)('\t')
			.print((uint64_t)(events * rate_frac)).lf()
		;
	}
}

stat_t::stat_t(
	method_t const &_method, times_t const &_times, bool _hrr_flag
) :
	method(_method), times(_times), hrr_flag(_hrr_flag), tcount(), spinlock() {

	size_t _maxi = method.maxi();
	descr_t const *descr[_maxi + 1];

	for(size_t i = 0; i < _maxi; ++i)
		descr[i] = method.descr(i);

	descr[_maxi] = &times;

	res = new res_t(_maxi + 1, descr);

	try {
		res_sum = new res_t(_maxi + 1, descr);
	}
	catch(...) {
		delete res;
		throw;
	}

	times_ind = _maxi;
}

stat_t::~stat_t() throw() {
	delete res_sum;
	delete res;
}

void stat_t::update(size_t index, size_t value) {
	res->update(index, value);
}

void stat_t::update_time(interval_t time_real, interval_t time_events) {
	res->update_time(time_real, time_events);
	res->update(times_ind, times.index(time_real));
}

void stat_t::update_size(sizeval_t size_in, sizeval_t size_out) {
	res->update_size(size_in, size_out);
}

void stat_t::event() {
	res->event();
}

void stat_t::print(out_t &out, bool clear) {
	res_t *res_new = new res_t(*res_sum, false);

	{
		thr::spinlock_guard_t guard(spinlock);
		res_t *res_tmp = res;
		res = res_new;
		res_new = res_tmp;
	}

	(*res_sum) += (*res_new);

	(clear ? res_new : res_sum)->print(out, hrr_flag);

	delete res_new;

	size_t tc = tcount.get(clear);

	if(hrr_flag)
		out(CSTR("tasks: ")).print(tc).lf();
	else
		out(CSTR("tasks"))('\t').print(tc).lf();

	method.stat(out, clear, hrr_flag);
}

size_t times_t::index(interval_t t) const {
	if(t >= steps[0]) return 0;
	if(t < steps[size - 1]) return(size);

	size_t left = 1, right = size - 1;

	while(left <= right) {
		size_t i = (left + right) / 2;

		if(t >= steps[i - 1])
			right = i;
		else if(t < steps[i])
			left = i + 1;
		else
			return i;
	}

	fatal("internal error");
}

size_t times_t::value_max() const {
	return size;
}

void times_t::print_header(out_t &out) const {
	out(CSTR("times"));
}

void times_t::print_value(out_t &out, size_t value) const {
	if(value < size) {
		out.print(steps[value]);
	}
	out(' ')('-')('-')(' ');
	if(value) {
		out.print(steps[value - 1]);
	}
}

class times_simple_t : public times_t {
public:
	struct config_t {
		interval_t max, min;
		sizeval_t steps;

		inline config_t() :
			max(interval_second), min(interval_millisecond), steps(30) { }

		inline void check(in_t::ptr_t const &ptr) const {
			if(min <= interval_zero)
				config::error(ptr, "min must be a positive interval");

			if(max <= min)
				config::error(ptr, "max must be greater than min");

			if(!steps)
				config::error(ptr, "steps must be a positive number");

			if(steps >= sizeval_kilo)
				config::error(ptr, "steps is too big");
		}
	};

	inline times_simple_t(string_t const &, config_t const &config) :
		times_t(config.steps + 1) {
		double l_max = __builtin_log((config.max / interval_microsecond) + 0.);
		double l_min = __builtin_log((config.min / interval_microsecond) + 0.);

		size_t n = size - 1;

		for(size_t i = 0; i <= n; ++i)
			steps[i] = interval_microsecond * __builtin_llrint(__builtin_exp((i * l_min + (n - i) * l_max) / n));
	}

	inline ~times_simple_t() throw() { }
};

namespace times_simple {
config_binding_sname(times_simple_t);
config_binding_value(times_simple_t, max);
config_binding_value(times_simple_t, min);
config_binding_value(times_simple_t, steps);
config_binding_ctor(times_t, times_simple_t);
}

class times_list_t : public times_t {
public:
	struct config_t {
		config::list_t<interval_t> values;

		inline config_t() { }

		inline void check(in_t::ptr_t const &ptr) const {
			interval_t last = interval_zero;

			for(typeof(values.ptr()) _ptr = values; _ptr; ++_ptr) {
				interval_t t = _ptr.val();

				if(t <= last)
					config::error(ptr, "values is not monotonic nonzero list");

				last = t;
			}

			if(last == interval_zero)
				config::error(ptr, "values is empty");
		}

		inline size_t size() const {
			size_t res = 0;

			for(typeof(values.ptr()) _ptr = values; _ptr; ++_ptr)
				++res;

			return res;
		}
	};

	inline times_list_t(string_t const &, config_t const &config) :
		times_t(config.size()) {

		size_t i = size;

		for(typeof(config.values.ptr()) ptr = config.values; ptr; ++ptr)
			steps[--i] = ptr.val();
	}

	inline ~times_list_t() throw() { }
};

namespace times_list {
config_binding_sname(times_list_t);
config_binding_value(times_list_t, values);
config_binding_ctor(times_t, times_list_t);
}

}} // namespace phantom::io_benchmark
