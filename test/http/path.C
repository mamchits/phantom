#include <pd/http/server.H>

#include <pd/base/out_fd.H>

#include <stdio.h>

using namespace pd;

static char outbuf[1024];
static out_fd_t out(outbuf, sizeof(outbuf), 1);

class backend_test_t : public log::backend_t {
	virtual void commit(iovec const *iov, size_t count) const throw();
	virtual log::level_t level() const throw();

public:
	inline backend_test_t() throw() { }
	inline ~backend_test_t() throw() { }
};

log::level_t backend_test_t::level() const throw() { return log::debug; }

void backend_test_t::commit(iovec const *iov, size_t count) const throw() {
	while(count--) {
		out(str_t((char const *)iov->iov_base, iov->iov_len));
		++iov;
	}
}

static backend_test_t backend_test;

class handler_test_t : log::handler_base_t {
	virtual void vput(
		log::level_t level, log::aux_t const *aux, char const *format, va_list vargs
	) const throw();

public:
	inline handler_test_t() throw() :
		log::handler_base_t(STRING("test"), &backend_test, true) { }

	inline ~handler_test_t() throw() { }
};

void handler_test_t::vput(
	log::level_t, log::aux_t const *aux, char const *format, va_list args
) const throw() {
	int __errno = errno;
	char buf[1024];
	size_t size = vsnprintf(buf, sizeof(buf), format, args);
	minimize(size, sizeof(buf));
	out(str_t(buf, size)).lf();
	if(aux) {
		// aux->print(out);
		out(CSTR("some aux info present")).lf();
	}

	out.flush_all();
	errno = __errno;
}

static handler_test_t const handler_test;

// ---------------------------------------------------------------

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
