#include <pd/base/str.H>
#include <pd/base/out_fd.H>

using namespace pd;

static char outbuf[1024];
static out_fd_t out(outbuf, sizeof(outbuf), 1);

extern "C" int main() {
	out.print(CSTR("qq")).lf();
	out.print(CSTR("qq\n\a\033"), "e").lf();
	out.print(CSTR("\0\015\016\\\"123qq\n\a\03344\177"), "e").lf();
	out.print(CSTR("абабагаламага"), "e").lf();
	out.print(CSTR("абабагаламага"), "e8").lf();
	out.print(CSTR("\0\015\016\\\"123qq\n\a\03344\177"), "j").lf();
	out.print(CSTR("абабагаламага"), "j").lf();
	out.print(CSTR("абабагаламага"), "j8").lf();
}
