// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "time.H"
#include "config_helper.H"

#include <time.h>

namespace pd {

static void construct_from_tm(timeval_t const &timeval, timestruct_t &ts) {
	interval_t tvv = timeval - timeval::unix_origin;

	ts.microseconds = (tvv % interval::second) / interval::microsecond;

	time_t unix_time = tvv / interval::second;

	struct tm unix_tm;

	// FIXME: Don't use libc localtime!

	localtime_r(&unix_time, &unix_tm);

	ts.tz_offset = unix_tm.tm_gmtoff;

	ts.second = unix_tm.tm_sec;
	ts.minute = unix_tm.tm_min;
	ts.hour = unix_tm.tm_hour;

	ts.day = unix_tm.tm_mday - 1;
	ts.month = unix_tm.tm_mon;
	ts.wday = (unix_tm.tm_wday + 5) % 6;
	ts.year = 1899 + unix_tm.tm_year;
}

constexpr interval_t interval_400_years((400 * 365 + 97) * interval::day);

timestruct_t::timestruct_t(timeval_t const &timeval, bool local) throw() {
	if(local) {
		construct_from_tm(timeval, *this);
		return;
	}

	tz_offset = 0;

	interval_t tvv = timeval - timeval::epoch_origin;

	int years_delta = 0;

	if(tvv < interval::zero) {
		years_delta = ((-tvv) / interval_400_years + 1) * 400;
		tvv = interval_400_years - (-tvv) % interval_400_years;
	}

	microseconds = (tvv % interval::second) / interval::microsecond;

	int64_t tv = tvv / interval::second;

	unsigned int d = tv / (60 * 60 * 24);
	unsigned int t = tv % (60 * 60 * 24);

	second = t % 60; t /= 60;
	minute = t % 60; t /= 60;
	hour = t;

	year = 0;

	wday = d % 7;

	bool flag = false;
	unsigned c = d / (400 * 365 + 97);
	year += c * 400; d -= c * (400 * 365 + 97);

	flag = (!flag && d >= 3 * (100 * 365 + 24));
	c = flag ? 3 : (d / (100 * 365 + 24));
	year += c * 100; d -= c * (100 * 365 + 24);

	flag = (!flag && (d >= 24 * (4 * 365 + 1)));
	c = flag ? 24 : (d / (4 * 365 + 1));
	year += c * 4; d -= c * (4 * 365 + 1);

	flag = (!flag && (d >= 3 * 365));
	c = flag ? 3 : (d / 365);
	year += c; d -= c * 365;

	year -= years_delta;

	if(flag) {
		switch(d) {
			case   0 ...  30: d -=   0; month =  0; break;
			case  31 ...  59: d -=  31; month =  1; break;
			case  60 ...  90: d -=  60; month =  2; break;
			case  91 ... 120: d -=  91; month =  3; break;
			case 121 ... 151: d -= 121; month =  4; break;
			case 152 ... 181: d -= 152; month =  5; break;
			case 182 ... 212: d -= 182; month =  6; break;
			case 213 ... 243: d -= 213; month =  7; break;
			case 244 ... 273: d -= 244; month =  8; break;
			case 274 ... 304: d -= 274; month =  9; break;
			case 305 ... 334: d -= 305; month = 10; break;
			case 335 ... 365: d -= 335; month = 11; break;
		}
	} else {
		switch(d) {
			case   0 ...  30: d -=   0; month =  0; break;
			case  31 ...  58: d -=  31; month =  1; break;
			case  59 ...  89: d -=  59; month =  2; break;
			case  90 ... 119: d -=  90; month =  3; break;
			case 120 ... 150: d -= 120; month =  4; break;
			case 151 ... 180: d -= 151; month =  5; break;
			case 181 ... 211: d -= 181; month =  6; break;
			case 212 ... 242: d -= 212; month =  7; break;
			case 243 ... 272: d -= 243; month =  8; break;
			case 273 ... 303: d -= 273; month =  9; break;
			case 304 ... 333: d -= 304; month = 10; break;
			case 334 ... 364: d -= 334; month = 11; break;
		}
	}

	day = d;
}

bool timestruct_t::mk_timeval(timeval_t &tv) const throw() {
	static unsigned int month_origin[2][13] = {
		{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
		{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
	};

	//if(year >= 292000) return false;

	interval_t delta = interval::zero;

	int _year = year;

	if(_year < 0) {
		delta = ((-_year) / 400 + 1) * interval_400_years;
		_year = 400 - (-_year) % 400;
	}

	unsigned int res = _year * 365 + (_year / 4) - (_year / 100) + (_year / 400);
	bool flag = (_year % 4 == 3 && (_year % 100 != 99 || _year % 400 == 399));

	if(month >= 12) return false;
	unsigned int const *m_origin = month_origin[flag];
	res += m_origin[month];

	if(day >= (m_origin[month] - m_origin[month + 1])) return false;
	res += day;

	if(wday < 8 && wday != res % 7) return false;

	if(hour >= 24 || minute >= 60 || second >= 60) return false;

	tv = timeval::epoch_origin +
		res * interval::day +
		hour * interval::hour +
		minute * interval::minute +
		second * interval::second +
		microseconds * interval::microsecond -
		tz_offset * interval::second -
		delta
	;

	return true;
}

template<>
void out_t::helper_t<timeval_t>::print(
	out_t &out, timeval_t const &tv, char const *fmt
) {
	bool local = false;
	bool date = false;
	bool offset = false;
	int msec = 0;

	if(fmt) {
		char c;
		while((c = *(fmt++))) {
			switch(c) {
				case '+': local = true; break;
				case 'd': date = true; break;
				case 'z': offset = true; break;
				case '.': ++msec; break;
			}
		}
	}

	timestruct_t ts(tv, local);

	if(date) {
		if(ts.year >= 0)
			out.print(ts.year + 1)('-');
		else
			out.print(-ts.year)(CSTR("BCE-"));

		out
			.print(ts.month + 1, "02")('-')
			.print(ts.day + 1, "02")(' ')
		;
	}

	out
		.print(ts.hour, "02")(':')
		.print(ts.minute, "02")(':')
		.print(ts.second, "02")
	;

	if(msec == 1) {
		out('.').print(ts.microseconds / 1000, "03");
	}
	else if(msec > 1) {
		out('.').print(ts.microseconds, "06");
	}

	if(offset) {
		int off = ts.tz_offset / 60;

		out(' ');

		if(off < 0) {
			out('-'); off = -off;
		}
		else {
			out('+');
		}

		out.print(off / 60, "02").print(off % 60, "02");
	}
}

template<>
void out_t::helper_t<interval_t>::print(
	out_t &out, interval_t const &_i, char const *fmt
) {
	unsigned short int prec = 10;

	if(fmt) {
		if(*fmt == '.') {
			prec = 0;
			++fmt;
			while(*fmt >= '0' && *fmt <= '9') (prec *= 10) += (*(fmt++) - '0');
		}
		if(!prec) prec = 10;
	}

	interval_t i = _i;

	if(i < interval::zero) {
		out('-');
		i = -i;
	}

	if(!i.is_real()) {
		out(CSTR("inf"));
	}
	else if(i < interval::millisecond) {
		out('0');
	}
	else {
		bool flag = false;
		if(i >= interval::week) {
			out.print((uint32_t)(i / interval::week))('w');
			i %= interval::week;

			flag = true;
		}

		if(flag && !--prec)
			return;

		if(i >= interval::day) {
			out.print((uint8_t)(i / interval::day))('d');
			i %= interval::day;

			flag = true;
		}

		if(flag && !--prec)
			return;

		if(i >= interval::hour) {
			out.print((uint8_t)(i / interval::hour))('h');
			i %= interval::hour;

			flag = true;
		}

		if(flag && !--prec)
			return;

		if(i >= interval::minute) {
			out.print((uint8_t)(i / interval::minute))('m');
			i %= interval::minute;

			flag = true;
		}

		if(flag && !--prec)
			return;

		if(i >= interval::second) {
			out.print((uint8_t)(i / interval::second))('s');
			i %= interval::second;

			flag = true;
		}

		if(flag && !--prec)
			return;

		if(i >= interval::millisecond) {
			unsigned short n = i / interval::millisecond;
			out('0' + n / 100); n %= 100;
			out('0' + n / 10); n %= 10;
			out('0' + n);
		}
	}
}

namespace config {

static inline bool is_digit(char c) { return c >= '0' && c <= '9'; }

template<>
void helper_t<interval_t>::parse(in_t::ptr_t &ptr, interval_t &interval) {
	in_t::ptr_t p = ptr;
	bool first = true;
	bool term = false;

	interval_t itmp, isum;

	// We dont parse negative intervals

	if(p.match<ident_t>(CSTR("inf"))) {
		interval = interval::inf;
	}
	else {
		while(!term) {
			if(!(p && is_digit(*p))) {
				if(first)
					error(p, "digit is expected");
				else
					break;
			}

			first = false;

			uint64_t ures;
			helper_t<uint64_t>::parse(p, ures);
			int64_t res = ures;
			if(res < 0)
				error(ptr, "integer overflow");

			switch(p ? *p : '\0') {
				case 's':
					itmp = res * interval::second;
					if(itmp / interval::second != res) goto over;
					++p;
					break;
				case 'm':
					itmp = res * interval::minute;
					if(itmp / interval::minute != res) goto over;
					++p;
					break;
				case 'h':
					itmp = res * interval::hour;
					if(itmp / interval::hour != res) goto over;
					++p;
					break;
				case 'd':
					itmp = res * interval::day;
					if(itmp / interval::day != res) goto over;
					++p;
					break;
				case 'w':
					itmp = res * interval::week;
					if(itmp / interval::week != res) goto over;
					++p;
					break;
				default:;
					itmp = res * interval::millisecond;
					term = true;
			}
			if(isum + itmp < isum) goto over;
			isum += itmp;
		}

		interval = isum;
	}

	ptr = p;
	return;

over:
	error(ptr, "integer overflow");
}

template<>
void helper_t<interval_t>::print(out_t &out, int, interval_t const &interval) {
	out.print(interval);
}

template<>
void helper_t<interval_t>::syntax(out_t &out) {
	out(CSTR("<interval>"));
}

} // namespace config

} // namespace pd
