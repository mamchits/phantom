#include <pd/base/time.H>
#include <pd/base/out_fd.H>
#include <pd/base/random.H>

#include <stdlib.h>

using namespace pd;

static char obuf[1024];
static out_fd_t out(obuf, sizeof(obuf), 1);

static void __test(timeval_t const &tv) {
	timestruct_t ts(tv);
	timeval_t __tv;

	out
		.print(ts.year)(' ')
		.print(ts.month)(' ')
		.print(ts.day)(' ')
		.print(ts.wday)(' ')
		.print(ts.hour)(' ')
		.print(ts.minute)(' ')
		.print(ts.second)(' ')
		.print(ts.microseconds)(' ')
		.print(ts.tz_offset).lf()
	;

	out.print(tv, "d").lf();

	if(ts.mk_timeval(__tv)) {
		if(__tv != tv)
			out(CSTR("ERROR1")).lf();
	}
	else {
		out(CSTR("ERROR2")).lf();
	}

	out.lf();
}

#define test(x) \
	out(CSTR(#x)).lf(); \
	__test(x);

extern "C" int main() {
	test(timeval_epoch_origin);

	for(int i = -500; i <= 2000; i += 150) {
		test(timeval_unix_origin + i * interval_week);
	}

	// test(timeval_never - (timeval_unix_origin - timeval_epoch_origin));

	test(timeval_epoch_origin - interval_millisecond);
	test(timeval_epoch_origin + (400 * 365 + 97) * interval_day);
	test(timeval_epoch_origin - 2 * (400 * 365 + 97) * interval_day);

	test(timeval_long_ago);
	test(timeval_never);

	out
		.lf()
		.print(interval_week).lf()
		.print(interval_day).lf()
		.print(interval_hour).lf()
		.print(interval_minute).lf()
		.print(interval_second).lf()
		.print(interval_millisecond).lf()
		.print(interval_second * 31 + interval_millisecond * 556).lf()
		.print(interval_inf).lf()
		.print(interval_inf / 2 + interval_millisecond).lf()
		.print(interval_inf / 2 - interval_millisecond).lf()
		.print(interval_week - interval_inf).lf()
	;

	out.lf();

	timeval_t t = timeval_unix_origin + 5109848497262789ULL * interval_microsecond;

	out.print(t, "d").lf();
	out.print(t, "d.").lf();
	out.print(t, "d..").lf();

	return 0;
}
