// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/defs.H>
#include <pd/base/op.H>

#include <stdint.h>
#include <sys/time.h>

#pragma GCC visibility push(default)

namespace pd {

struct interval_t : dim_t<interval_t, int64_t> {
	inline constexpr interval_t() = default;

	inline explicit constexpr interval_t(int64_t _val) throw() :
		dim_t<interval_t, int64_t>(_val) { }

	inline constexpr interval_t(interval_t const &) = default;
	inline interval_t &operator=(interval_t const &) = default;
	inline ~interval_t() = default;

	inline bool is_real() const throw();

	static inline constexpr interval_t max() { return interval_t((~(uint64_t)0) >> 2); }
	static inline constexpr interval_t min() { return -max(); }
};

namespace interval {

inline constexpr interval_t from_microseconds(__suseconds_t x) {
	return interval_t(x);
}

constexpr interval_t zero(0);
constexpr interval_t microsecond(1);
constexpr interval_t millisecond(microsecond * 1000);
constexpr interval_t second(millisecond * 1000);
constexpr interval_t minute(second * 60);
constexpr interval_t hour(minute * 60);
constexpr interval_t day(hour * 24);
constexpr interval_t week(day * 7);
constexpr interval_t inf(interval_t::max());

} // namespace interval

inline bool interval_t::is_real() const throw() {
	return ((*this) < +interval::inf / 2) && ((*this) > -interval::inf / 2);
}

struct interval_sq_t : dim_t<interval_sq_t, int64_t> {
	inline constexpr interval_sq_t() = default;

	inline explicit constexpr interval_sq_t(int64_t _val) throw() :
		dim_t<interval_sq_t, int64_t>(_val) { }

	inline constexpr interval_sq_t(interval_sq_t const &) = default;
	inline interval_sq_t &operator=(interval_sq_t const &) = default;
	inline ~interval_sq_t() = default;
};

inline interval_sq_t operator*(
	interval_t const &i1, interval_t const &i2
) {
	return interval_sq_t(((i1.val() + 500) / 1000) * ((i2.val() + 500) / 1000));
}

inline interval_t operator/(
	interval_sq_t const &isq, interval_t const &i
) {
	int64_t isqval = isq.val(), ival = i.val();

	int64_t mult = 1000000;

	while(mult > 1) {
		int64_t q = isqval * mult;
		if(q / mult == isqval)
			return interval_t((q / ival) * (1000000 / mult));

		mult /= 10;
	}

	return interval_t((isqval / ival) * 1000000);
}

class timeval_t : public rel_ops_t<timeval_t> {
	int64_t val;

public:
	inline explicit constexpr timeval_t(int64_t const &_val) throw() : val(_val) { }

	inline constexpr timeval_t() throw() : val(0) { }
	inline constexpr timeval_t(timeval_t const &num) throw() : val(num.val) { }
	~timeval_t() = default;

	inline timeval_t &operator=(timeval_t const &v) throw() {
		val = v.val; return *this;
	}

	inline timeval_t &operator+=(interval_t const &i) throw() {
		val += i.val(); return *this;
	}

	inline constexpr timeval_t operator+(interval_t const &i) const throw() {
		return timeval_t(val + i.val());
	}

	inline timeval_t &operator-=(interval_t const &i) throw() {
		val -= i.val(); return *this;
	}

	inline constexpr timeval_t operator-(interval_t const &i) const throw() {
		return timeval_t(val - i.val());
	}

	friend inline constexpr bool operator==(
		timeval_t const &v1, timeval_t const &v2
	) throw() {
		return v1.val == v2.val;
	}

	friend inline constexpr bool operator<(
		timeval_t const &v1, timeval_t const &v2
	) throw() {
		return v1.val < v2.val;
	}

	friend inline constexpr interval_t operator-(
		timeval_t const &v1, timeval_t const &v2
	) throw() {
		return interval_t(v1.val - v2.val);
	}

	inline bool is_real() const throw() {
		return
			((*this) < timeval_t(0) + interval::inf / 2) &&
			((*this) > timeval_t(0) - interval::inf / 2)
		;
	}
};

namespace timeval {

constexpr timeval_t unix_origin(0);
constexpr timeval_t epoch_origin(unix_origin - 719162 * interval::day);

constexpr timeval_t never(unix_origin + interval::inf);
constexpr timeval_t long_ago(unix_origin - interval::inf);

static inline timeval_t current() throw() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return
		unix_origin +
		(tv.tv_sec * interval::second) +
		interval::from_microseconds(tv.tv_usec)
	;
}

} // namespace timeval

struct timestruct_t {
	int year;
	unsigned int month, day, wday;
	unsigned int hour, minute, second;
	unsigned int microseconds;
	int tz_offset;

	inline timestruct_t() throw() : wday(8) { }
	timestruct_t(timeval_t const &, bool local = false) throw();

	bool mk_timeval(timeval_t &tv) const throw();
};

} // namespace pd

#pragma GCC visibility pop
