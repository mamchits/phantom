#include <pd/base/str.H>
#include <pd/base/out_fd.H>

using namespace pd;

static char outbuf[1024];
static out_fd_t out(outbuf, sizeof(outbuf), 1);

template<size_t _n> struct q_t { static inline size_t n() { return _n; } };

extern "C" int main() {
	out.print(CSTR("qq")).lf();
	out.print(CSTR("qq\n\a\033"), "e").lf();
	out.print(CSTR("\0\015\016\\\"123qq\n\a\03344\177"), "e").lf();
	out.print(CSTR("абабагаламага"), "e").lf();
	out.print(CSTR("абабагаламага"), "e8").lf();
	out.print(CSTR("\0\015\016\\\"123qq\n\a\03344\177"), "j").lf();
	out.print(CSTR("абабагаламага"), "j").lf();
	out.print(CSTR("абабагаламага"), "j8").lf();


	bool ok = true;
	
	ok = ok && q_t<str_t("abcd").size()>::n() == 4;
	ok = ok && q_t<str_t("zxcvb", 2).size()>::n() == 2;

	ok = ok && q_t<cstrlen(str_t("qwertp").ptr())>::n() == 6;
	ok = ok && q_t<str_t("") ? 6 : 7>::n() == 7;

	constexpr str_t str("asdfgh");

	ok = ok && q_t<str.size()>::n() == 6;

	return ok ? 0 : 1;
}
