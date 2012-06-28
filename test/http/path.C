#include <pd/http/server.H>

#include <pd/base/out_fd.H>

using namespace pd;

static void path_decode_test(string_t const &path) {
	char buf[1024];
	out_fd_t out(buf, sizeof(buf), 1);

	try {
		string_t dpath = http::path_decode(path);

		out('\"')(path)(CSTR("\" -> \""))(dpath)('\"').lf();
	}
	catch(...) {
		out('\"')(path)(CSTR("\" -> Error")).lf();
	}
}


#define _(x) path_decode_test(STRING(x));

extern "C" int main() {
	_("/");
	_("/./");
	_("/../");
	_("/.");
	_("/..");
	_("/abaaa/ccca/dddd/../qqqqq/../pqqr");
	_("/abaaa/ccca/dddd/../qqqqq/../../pqqr");
	_("/abaaa/ccca/dddd/./qqqqq/././pqqr");
	_("/abaaa/ccca/dddd//qqqqq///pqqr");
	_("/abaaa/ccca/dddd//qqqqq///pqqr/.././../..");
	_("/abaaa/ccca/dddd/../qqqqq%2F%2E%2E%2F%2E%2E%2Fpqqr");
	_("/%2F%2E%2E%2F%2E%2E%2Fpqqr");
	_("/%pqqr");
	_("/%");
	_("/.ewrqewr/qweeee/");
	_("/wwww/..qweqw/qqq");
	return 0;
}
