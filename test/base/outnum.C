#include <pd/base/out_fd.H>

#include <limits.h>

using namespace pd;

extern "C" int main() {
	char obuf[1024];
	out_fd_t out(obuf, sizeof(obuf), 1);

	out.print((signed char)(SCHAR_MAX)).lf();
	out.print((signed char)SCHAR_MIN).lf();
	out.lf();
	out.print((short)(SHRT_MAX)).lf();
	out.print((short)SHRT_MIN).lf();
	out.lf();
	out.print((int)(INT_MAX)).lf();
	out.print((int)INT_MIN).lf();
	out.lf();
#ifdef __64bit
	out.print((long)(LONG_MAX)).lf();
	out.print((long)LONG_MIN).lf();
	out.lf();
#else
	out.print((long long)(LONG_LONG_MAX)).lf();
	out.print((long long)LONG_LONG_MIN).lf();
	out.lf();
#endif
	out.print('\377').lf();
	out.print((unsigned char)'\377').lf();
	out.print((signed char)'\377').lf();

	return 0;
}
