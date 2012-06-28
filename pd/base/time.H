// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/op.H>

#include <stdint.h>
#include <sys/time.h>

#pragma GCC visibility push(default)

namespace pd {

class interval_t : public rel_ops_t<interval_t> {
	int64_t val;

public:
	explicit inline interval_t(int64_t _val) throw() : val(_val) { }

	inline interval_t() throw() : val(0) { }
	inline interval_t(const interval_t &i) throw() : val(i.val) { }
	inline ~interval_t() throw() { }

	inline interval_t &operator=(interval_t const &i) throw() {
		val = i.val; return *this;
	}

	friend inline interval_t operator-(interval_t const &i) throw() {
		return interval_t(-i.val);
	}

	inline interval_t &operator+=(interval_t const &i) throw() {
		val += i.val; return *this;
	}

	friend inline interval_t operator+(
		interval_t const &i1, interval_t const &i2
	) throw() {
		return interval_t(i1.val + i2.val);
	}

	inline interval_t &operator-=(interval_t const &i) throw() {
		val -= i.val; return *this;
	}

	friend inline interval_t operator-(
		interval_t const &i1, interval_t const &i2
	) throw() {
		return interval_t(i1.val - i2.val);
	}

	template <typename x_t>
	inline interval_t &operator*=(x_t const &x) throw() {
		val *= x; return *this;
	}

	template <typename x_t>
	friend inline interval_t operator*(
		interval_t const &i, x_t const &x
	) throw() {
		return interval_t((int64_t)(i.val * x));
	}

	template <typename x_t>
	friend inline interval_t operator*(
		x_t const &x, interval_t const &i
	) throw() {
		return interval_t((int64_t)(x * i.val));
	}

	template <typename x_t>
	inline interval_t &operator/=(x_t const &x) throw() {
		val /= x; return *this;
	}

	template <typename x_t>
	friend inline interval_t operator/(
		interval_t const &i, x_t const &x
	) throw() {
		return interval_t((int64_t)(i.val / x));
	}

	friend inline int64_t operator/(
		interval_t const &i1, interval_t const &i2
	) throw() {
		return i1.val / i2.val;
	}

	inline interval_t &operator%=(interval_t const &i) throw() {
		val %= i.val; return *this;
	}

	friend inline interval_t operator%(
		interval_t const &i1, interval_t const &i2
	) throw() {
		return interval_t(i1.val % i2.val);
	}

	friend inline bool operator==(
		interval_t const &i1, interval_t const &i2
	) throw() {
		return i1.val == i2.val;
	}

	friend inline bool operator<(
		interval_t const &i1, interval_t const &i2
	) throw() {
		return i1.val < i2.val;
	}

	friend class timeval_t;

	//inline operator bool() const { return val != 0; }

#define interval_microsecond_val ((int64_t)1)
#define interval_millisecond_val (interval_microsecond_val * 1000)
#define interval_second_val (interval_millisecond_val * 1000)
#define interval_minute_val (interval_second_val * 60)
#define interval_hour_val (interval_minute_val * 60)
#define interval_day_val (interval_hour_val * 24)
#define interval_week_val (interval_day_val * 7)
#define interval_inf_val ((int64_t)(((~0ULL) >> 2)))

#define interval_from_microseconds(x) interval_t(x)
#define interval_zero interval_t(0)
#define interval_microsecond interval_t(interval_microsecond_val)
#define interval_millisecond interval_t(interval_millisecond_val)
#define interval_second interval_t(interval_second_val)
#define interval_minute interval_t(interval_minute_val)
#define interval_hour interval_t(interval_hour_val)
#define interval_day interval_t(interval_day_val)
#define interval_week interval_t(interval_week_val)
#define interval_inf interval_t(interval_inf_val)

	inline bool is_real() const throw() {
		return
			((*this) < interval_inf / 2) &&
			((*this) > - interval_inf / 2)
		;
	}

private:
	//inline operator int64_t() const;
};

class timeval_t : public rel_ops_t<timeval_t> {
	int64_t val;
public:
	explicit inline timeval_t(int64_t const &_val) throw() : val(_val) { }

	inline timeval_t() throw() : val(0) { }
	inline timeval_t(timeval_t const &num) throw() : val(num.val) { }
	inline ~timeval_t() throw() { }

	inline timeval_t &operator=(timeval_t const &v) throw() {
		val = v.val; return *this;
	}

	inline timeval_t &operator+=(interval_t const &i) throw() {
		val += i.val; return *this;
	}

	inline timeval_t operator+(interval_t const &i) const throw() {
		return timeval_t(val + i.val);
	}

	inline timeval_t &operator-=(interval_t const &i) throw() {
		val -= i.val; return *this;
	}

	inline timeval_t operator-(interval_t const &i) const throw() {
		return timeval_t(val - i.val);
	}

	friend inline bool operator==(
		timeval_t const &v1, timeval_t const &v2
	) throw() {
		return v1.val == v2.val;
	}

	friend inline bool operator<(
		timeval_t const &v1, timeval_t const &v2
	) throw() {
		return v1.val < v2.val;
	}

	friend inline interval_t operator-(
		timeval_t const &v1, timeval_t const &v2
	) throw() {
		return interval_t(v1.val - v2.val);
	}

#define timeval_unix_origin timeval_t(0)
#define timeval_epoch_origin timeval_t(interval_day_val * -719162)

#define timeval_never (timeval_t(0) + interval_inf)
#define timeval_long_ago (timeval_t(0) - interval_inf)

	inline bool is_real() const throw() {
		return
			((*this) < timeval_t(0) + interval_inf / 2) &&
			((*this) > timeval_t(0) - interval_inf / 2)
		;
	}
};

static inline timeval_t timeval_current() throw() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return
		timeval_unix_origin +
		(tv.tv_sec * interval_second) +
		interval_from_microseconds(tv.tv_usec)
	;
}

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
