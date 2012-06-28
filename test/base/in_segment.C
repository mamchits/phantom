#include <pd/base/string.H>
#include <pd/base/in_fd.H>
#include <pd/base/out_fd.H>
#include <pd/base/fd_guard.H>
#include <pd/base/exception.H>

#include <unistd.h>
#include <fcntl.h>

using namespace pd;

static char outbuf[1024];
static out_fd_t out(outbuf, sizeof(outbuf), 1);

#define check(expr) \
	out(CSTR("(" #expr "): ")); \
	out((expr) ? CSTR("Ok") : CSTR("Fail")).lf()

static in_segment_t readpipe(int fd) {
	fd_guard_t guard(fd);

	in_fd_t in(7, fd);

	in_t::ptr_t p0 = in;

	in_t::ptr_t p = p0;

	while(p)
		p += p.pending();

	return in_segment_t(p0, p - p0);
}

static in_segment_t duplicate(in_segment_t const &segment) {
	in_segment_list_t dseg;
	dseg.append(segment);
	dseg.append(segment);
	return dseg;
}

static void print(in_segment_t const &seg) {
	out(CSTR("D: ")).print(seg.depth()).lf()(seg);
}

extern "C" int main() {
	int fds[2];
	if(::pipe(fds) < 0)
		fatal("cant create pipe");

	if(::fork() == 0) {
		char obuf[128];
		out_fd_t out(obuf, sizeof(obuf), fds[1]);
		out
			(CSTR("0123456789")).lf()
			(CSTR("abcdefghijklmnopqrstuvwxyz")).lf()
			(CSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ")).lf()
		;
		out.flush_all();
		return 0;
	}

	try {
		::close(fds[1]);
		in_segment_t seg0 = readpipe(fds[0]);
		print(seg0);

		in_segment_t seg1 = duplicate(seg0);
		print(seg1);

		in_segment_t seg2 = duplicate(seg1);
		print(seg2);

		in_segment_t seg3 = duplicate(seg2);
		print(seg3);

		check(seg3.size() == 8 * seg0.size());

		in_segment_t subseg1 = ({
			in_t::ptr_t p = seg3;
			p += 4 * seg0.size() + 11;
			in_segment_t(p, 2 * seg0.size());
		});

		print(subseg1);

		in_segment_t subseg2 = ({
			in_t::ptr_t p = seg3;
			p += 2 * seg0.size() + 11;
			in_segment_t(p, seg0.size());
		});

		print(subseg2);

		in_segment_t subseg3 = ({
			in_t::ptr_t p = seg3;
			p += 3 * seg0.size() + 11;
			in_segment_t(p, seg0.size() - 11);
		});

		print(subseg3);

		in_segment_list_t slist1;
		slist1.append(seg0);
		print(slist1);

		in_segment_list_t slist2;
		slist2.append(slist1);
		print(slist2);

		in_segment_t seg = slist2;
		print(seg);
	}
	catch(exception_t const &ex) {
		ex.log();
		return 1;
	}
	catch(...) {
		log_error("unknown exception");
		return 1;
	}

	return 0;
}
