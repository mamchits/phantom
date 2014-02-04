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
	test(timeval::epoch_origin);

	for(int i = -500; i <= 2000; i += 150) {
		test(timeval::unix_origin + i * interval::week);
	}

	// test(timeval::never - (timeval::unix_origin - timeval::epoch_origin));

	test(timeval::epoch_origin - interval::millisecond);
	test(timeval::epoch_origin + (400 * 365 + 97) * interval::day);
	test(timeval::epoch_origin - 2 * (400 * 365 + 97) * interval::day);

	test(timeval::long_ago);
	test(timeval::never);

	out
		.lf()
		.print(interval::week).lf()
		.print(interval::day).lf()
		.print(interval::hour).lf()
		.print(interval::minute).lf()
		.print(interval::second).lf()
		.print(interval::millisecond).lf()
		.print(interval::second * 31 + interval::millisecond * 556).lf()
		.print(interval::day * 2 + interval::hour * 5 + interval::minute * 16, ".2").lf()
		.print(interval::inf).lf()
		.print(interval::inf / 2 + interval::millisecond).lf()
		.print(interval::inf / 2 - interval::millisecond).lf()
		.print(interval::inf / 2 - interval::millisecond, ".4").lf()
		.print(interval::week - interval::inf).lf()
	;

	out.lf();

	timeval_t t = timeval::unix_origin + 5109848497262789ULL * interval::microsecond;

	out.print(t, "d").lf();
	out.print(t, "d.").lf();
	out.print(t, "d..").lf();

	return 0;
}
