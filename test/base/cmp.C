#include <pd/base/out_fd.H>
#include <pd/base/string.H>

using namespace pd;

static char outbuf[1024];
static out_fd_t out(outbuf, sizeof(outbuf), 1);

#define check(expr) \
	out(CSTR("(" #expr "): ")); \
	out((expr) ? CSTR("Ok") : CSTR("Fail")).lf()

extern "C" int main() {
	check(1 == 1);
	check(1 != 1);

	check(str_t::cmp<lower_t>(CSTR("/dOC"), CSTR("/doc")));
	check(str_t::cmp<lower_t>(CSTR("/doc"), CSTR("/doc")));
	check(!str_t::cmp<ident_t>(CSTR("/doc"), CSTR("/tmp")));
	check(!str_t::cmp<lower_t>(CSTR("/doc"), CSTR("/tmp")));
	check(str_t::cmp<ident_t>(CSTR("/doc1"), CSTR("/doc2")).is_less());
	check(str_t::cmp<lower_t>(CSTR("/doc1"), CSTR("/doc2")).is_less());
	check(str_t::cmp<ident_t>(CSTR("/doc"), CSTR("/doc2")).is_less());
	check(str_t::cmp<lower_t>(CSTR("/DOC"), CSTR("/doc2")).is_less());
	check(!str_t::cmp_eq<ident_t>(CSTR("/doc"), CSTR("/tmp")));
	check(!str_t::cmp_eq<lower_t>(CSTR("/doc"), CSTR("/tmp")));
	check(str_t::cmp_eq<ident_t>(CSTR("/doc"), CSTR("/doc")));
	check(str_t::cmp_eq<lower_t>(CSTR("/doc"), CSTR("/Doc")));
}
