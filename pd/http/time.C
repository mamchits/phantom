// This file is part of the pd::http library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "http.H"

namespace pd { namespace http {

static char const *(wnames[7]) = {
	"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

static char const *(mnames[12]) = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static inline void copy3(char const *src, char *dst) throw() {
	dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
}

static inline void dig2(unsigned int d, char *dst) throw() {
	dst[1] = '0' + (d % 10); d /= 10;
	dst[0] = '0' + (d % 10);
}

static inline void dig4(unsigned int d, char *dst) throw() {
	dst[3] = '0' + (d % 10); d /= 10;
	dst[2] = '0' + (d % 10); d /= 10;
	dst[1] = '0' + (d % 10); d /= 10;
	dst[0] = '0' + (d % 10);
}

struct time_current_cache_t {
	uint64_t last_time;
	unsigned long last_date;
	char buf[5 + 3 + 4 + 5 + 9 + 3];
};

static __thread time_current_cache_t time_current_cache = {
	0, 0,
	{
		'T', 'h', 'u', ',', ' ',
		'0', '1', ' ',
		'J', 'a', 'n', ' ',
		'1', '9', '7', '0', ' ',
		'0', '0', ':', '0', '0', ':', '0', '0', ' ',
		'G', 'M', 'T'
	}
};

string_t time_current_string(timeval_t &timeval_curr) {
	time_current_cache_t &cache = time_current_cache;

	timeval_t t = timeval::current();
	timeval_curr = t;

	uint64_t cache_time = (t - timeval::unix_origin) / interval::second;

	if(cache_time != cache.last_time) {
		unsigned long cache_date = cache_time / (60 * 60 * 24);

		if(cache_date != cache.last_date) {
			timestruct_t ts(t);

			copy3(wnames[ts.wday], cache.buf);
			dig2(ts.day + 1, cache.buf + 5);
			copy3(mnames[ts.month], cache.buf + 5 + 3);
			dig4(ts.year + 1, cache.buf + 5 + 3 + 4);

			cache.last_date = cache_date;
		}

		// (timeval::unix_origin - timeval::epch_origin) % (60 * 60 * 24) == 0

		unsigned int _time = cache_time % (60 * 60 * 24);

		dig2(_time % 60, cache.buf + 5 + 3 + 4 + 5 + 3 + 3); _time /= 60;
		dig2(_time % 60, cache.buf + 5 + 3 + 4 + 5 + 3); _time /= 60;
		dig2(_time, cache.buf + 5 + 3 + 4 + 5);

		cache.last_time = cache_time;
	}

	return
		string_t::ctor_t(sizeof(cache.buf))
			(str_t(cache.buf, sizeof(cache.buf)))
		;
}

string_t time_string(timeval_t time) {
	timestruct_t ts(time);

	unsigned int year = ts.year + 1;
	unsigned int day = ts.day + 1;

	return string_t::ctor_t(5 + 3 + 4 + 5 + 8 + 4)
		(str_t(wnames[ts.wday], 3))(',')(' ')                           // 5
		('0' + (day / 10) % 10)('0' + day % 10)(' ')                    // 3
		(str_t(mnames[ts.month], 3))(' ')                               // 4
		('0' + (year / 1000) % 10)('0' + (year / 100) % 10)
		('0' + (year / 10) % 10)('0' + year % 10)(' ')                  // 5
		('0' + (ts.hour / 10) % 10)('0' + ts.hour % 10)(':')
		('0' + (ts.minute / 10) % 10)('0' + ts.minute % 10)(':')
		('0' + (ts.second / 10) % 10)('0' + ts.second % 10)             // 8
		(CSTR(" GMT"))                                                  // 4
	;
}

static inline bool digit(char c, unsigned int &res) {
	if(c >= '0' && c <= '9') {
		res = c - '0'; return true;
	}
	return false;
}

template<unsigned int n>
class sp_t {
	char c[n];

public:
	inline sp_t(in_t::ptr_t &_ptr) {
		in_t::ptr_t _p = _ptr;
		for(unsigned int i = 0; i < n; ++i) {
			c[i] = *_p; ++_p;
		}
	}

	inline char operator[](unsigned int i) { return c[i]; }

	inline bool set_number(unsigned int &_num) {
		unsigned int num = 0;
		for(unsigned int i = 0; i < n; ++i) {
			unsigned int d;
			if(!digit(c[i], d))
				return false;
			num = num * 10 + d;
		}
		_num = num;
		return true;
	}

	inline bool set_number_SP(unsigned int &_num) {
		unsigned int num = 0;
		unsigned int i = 0;

		for(; i < n; ++i)
			if(c[i] != ' ') break;

		if(i == n)
			return false;

		for(; i < n; ++i) {
			unsigned int d;
			if(!digit(c[i], d))
				return false;
			num = num * 10 + d;
		}
		_num = num;
		return true;
	}
};

static bool time_parse_day(in_t::ptr_t &ptr, timestruct_t &ts) {
	sp_t<2> sp(ptr);

	unsigned int _day;
	if(!sp.set_number(_day) || !_day)
		return false;

	ptr += 2;
	ts.day = _day - 1;
	return true;
}

static bool time_parse_day_SP(in_t::ptr_t &ptr, timestruct_t &ts) {
	sp_t<2> sp(ptr);

	unsigned int _day;
	if(!sp.set_number_SP(_day) || !_day)
		return false;

	ptr += 2;
	ts.day = _day - 1;
	return true;
}

static bool time_parse_month(in_t::ptr_t &ptr, timestruct_t &ts) {
	sp_t<3> sp(ptr);

	switch(sp[0]) {
		case 'J':
			if(sp[1] == 'a' && sp[2] == 'n')
				ts.month = 0;
			else if(sp[1] == 'u') {
				if(sp[2] == 'n')
					ts.month = 5;
				else if(sp[2] == 'l')
					ts.month = 6;
				else
					return false;
			}
			else
				return false;

			break;
		case 'F':
			if(sp[1] == 'e' && sp[2] == 'b')
				ts.month = 1;
			else
				return false;

			break;
		case 'M':
			if(sp[1] == 'a') {
				if(sp[2] == 'r')
					ts.month = 2;
				else if(sp[2] == 'y')
					ts.month= 4;
				else
					return false;
			}
			else
				return false;

			break;
		case 'A':
			if(sp[1] == 'p' && sp[2] == 'r')
				ts.month = 3;
			else if(sp[1] == 'u' && sp[2] == 'g')
				ts.month = 7;
			else
				return false;

			break;
		case 'S':
			if(sp[1] == 'e' && sp[2] == 'p')
				ts.month = 8;
			else
				return false;

			break;
		case 'O':
			if(sp[1] == 'c' && sp[2] == 't')
				ts.month = 9;
			else
				return false;

			break;
		case 'N':
			if(sp[1] == 'o' && sp[2] == 'v')
				ts.month = 10;
			else
				return false;

			break;
		case 'D':
			if(sp[1] == 'e' && sp[2] == 'c')
				ts.month = 11;
			else
				return false;
	}

	ptr += 3;
	return true;
}

static bool time_parse_year(in_t::ptr_t &ptr, timestruct_t &ts) {
	sp_t<4> sp(ptr);

	unsigned int _year;
	if(!sp.set_number(_year) || !_year)
		return false;

	ptr += 4;
	ts.year = _year - 1;
	return true;
}

static bool time_parse_year_sh(in_t::ptr_t &ptr, timestruct_t &ts) {
	sp_t<2> sp(ptr);

	unsigned int _year;
	if(!sp.set_number(_year))
		return false;

	if(_year >= 70)
		_year += 1900;
	else
		_year += 2000;

	ptr += 2;
	ts.year = _year - 1;
	return true;
}

static inline bool two_digits(
	char c0, char c1, unsigned int lim, unsigned int &res
) throw() {
	unsigned int d0, d1;

	if(!digit(c0, d0) || !digit(c1, d1))
		return false;

	unsigned int _res = d0 * 10 + d1;
	if(_res >= lim) return false;

	res = _res;
	return true;
}

static bool time_parse_dtime(in_t::ptr_t &ptr, timestruct_t &ts) {
	sp_t<8> sp(ptr);

	unsigned int h, m, s;

	if(
		!two_digits(sp[0], sp[1], 24, h) || sp[2] != ':' ||
		!two_digits(sp[3], sp[4], 60, m) || sp[5] != ':' ||
		!two_digits(sp[6], sp[7], 60, s)
	)
		return false;

	ptr += 8;
	ts.hour = h; ts.minute = m; ts.second = s;
	return true;
}

static bool time_parse_wday(in_t::ptr_t &ptr, timestruct_t &ts) {
	sp_t<3> sp(ptr);

	switch(sp[0]) {
		case 'M':
			if(sp[1] == 'o' && sp[2] == 'n')
				ts.wday = 0;
			else
				return false;

			break;
		case 'T':
			if(sp[1] == 'u' && sp[2] == 'e')
				ts.wday = 1;
			else if(sp[1] == 'h' && sp[2] == 'u')
				ts.wday = 3;
			else
				return false;

			break;
		case 'W':
			if(sp[1] == 'e' && sp[2] == 'd')
				ts.wday = 2;
			else
				return false;

			break;
		case 'F':
			if(sp[1] == 'r' && sp[2] == 'i')
				ts.wday = 4;
			else
				return false;

			break;
		case 'S':
			if(sp[1] == 'a' && sp[2] == 't')
				ts.wday = 5;
			else if(sp[1] == 'u' && sp[2] == 'n')
				ts.wday = 6;
			else
				return false;

			break;
	}

	ptr += 3;
	return true;
}

bool time_parse(in_segment_t const &str, timeval_t &time) {
	in_t::ptr_t ptr = str;
	size_t len = str.size();
	timestruct_t ts;

	while(true) {
		if(len < 24) return false; // 24 -- smallest data length (asctime)

		char c = *ptr;
		if(c != ' ' && c != '\t' && c != '\r' && c != '\n') break;
		++ptr; --len;
	}

	if(!time_parse_wday(ptr, ts))
		return false;

	len -= 3;

	if(*ptr == ',') {
		++ptr; --len;
		if(*ptr != ' ') return false;

		++ptr; --len;

// RFC 2616, 3.3.1
// Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
// rfc1123-date = wkday "," SP date1 SP time SP "GMT"
// date1        = 2DIGIT SP month SP 4DIGIT

		if(
			len < 24 ||
			!time_parse_day(ptr, ts) || *(ptr++) != ' ' ||
			!time_parse_month(ptr, ts) || *(ptr++) != ' ' ||
			!time_parse_year(ptr, ts) || *(ptr++) != ' ' ||
			!time_parse_dtime(ptr, ts) || *(ptr++) != ' ' ||
			*(ptr++) != 'G' || *(ptr++) != 'M' || *(ptr++) != 'T'
		)
			return false;
	}
	else if(*ptr == ' ') {
		++ptr; --len;

// RFC 2616, 3.3.1
// Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
// asctime-date = wkday SP date3 SP time SP 4DIGIT
// date3        = month SP ( 2DIGIT | ( SP 1DIGIT ))

		if(
			len < 20 ||
			!time_parse_month(ptr, ts) || *(ptr++) != ' ' ||
			!time_parse_day_SP(ptr, ts) || *(ptr++) != ' ' ||
			!time_parse_dtime(ptr, ts) || *(ptr++) != ' ' ||
			!time_parse_year(ptr, ts)
		)
			return false;
	}
	else {
		switch(ts.wday) {
			case 5: // "Saturday"
				if(*(ptr++) != 'u' || *(ptr++) != 'r') return false;
				len -= 2;
				goto _day;
			case 2: // "Wednesday"
				if(*(ptr++) != 'n' || *(ptr++) != 'e') return false;
				len -= 2;
				goto _sday;
			case 3: //"Thursday"
				if(*(ptr++) != 'r') return false;
				--len;
			case 1: // "Tuesday"
			_sday:
				if(*(ptr++) != 's') return false;
				--len;
			case 0: case 4: case 6:
			_day:
				if(
					*(ptr++) != 'd' || *(ptr++) != 'a' || *(ptr++) != 'y' ||
					*(ptr++) != ',' || *(ptr++) != ' '
				) return false;
				len -= 5;
		}

// RFC 2616, 3.3.1
// Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
// rfc850-date  = weekday "," SP date2 SP time SP "GMT"
// date2        = 2DIGIT "-" month "-" 2DIGIT

		if(
			len < 22 ||
			!time_parse_day(ptr, ts) || *(ptr++) != '-' ||
			!time_parse_month(ptr, ts) || *(ptr++) != '-' ||
			!time_parse_year_sh(ptr, ts) || *(ptr++) != ' ' ||
			!time_parse_dtime(ptr, ts) || *(ptr++) != ' ' ||
			*(ptr++) != 'G' || *(ptr++) != 'M' || *(ptr++) != 'T'
		)
			return false;
	}

	ts.microseconds = 0;
	ts.tz_offset = 0;

	return ts.mk_timeval(time);
}

}} // namespace pd::http
