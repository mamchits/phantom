#include <pd/base/in.H>
#include <pd/base/out_fd.H>
#include <pd/base/log.H>
#include <pd/base/exception.H>

using namespace pd;

static char outbuf[1024];
static out_fd_t out(outbuf, sizeof(outbuf), 1);

void print(in_t &in) {
	in_t::ptr_t ptr = in;

	while(ptr) {
		out(*ptr);
		++ptr;
	}
	out.lf();
}

string_t const inbuf1[] = {
	STRING("....."),
	STRING("&&&&&"),
	string_t::empty
};

class page_buf_t : public in_t::page_t {
	size_t off_base;
	string_t const &data;

	virtual bool chunk(size_t off, str_t &str) const {
		char const *data_ptr = data.ptr();
		size_t data_size = data.size();

		if(off >= off_base && off < off_base + data_size) {
			str = str_t((data_ptr + (off - off_base)), (off_base - off) + data_size);

			return true;
		}
		return false;
	}

	virtual unsigned int depth() const { return 0; }

	virtual bool optimize(in_segment_t &) const { return false; }

	virtual ~page_buf_t() throw() { }

public:
	inline page_buf_t(size_t _off_base, string_t const &_data) :
		off_base(_off_base), data(_data) { }
};

class in_buf0_t : public in_t {
	virtual bool do_expand() { return false; }

	virtual void __noreturn unexpected_end() const {
		throw exception_log_t(log::error, "unexpected end of in_segment_t");
	}

public:
	inline in_buf0_t(string_t const *strs) : in_t() {
		size_t off = 0;

		ref_t<page_t> *last = &list;

		while(*strs) {
			*last = new page_buf_t(off, *strs);
			last = &((*last)->next);

			off += strs->size();
			++strs;
		}

		off_end = off;
	}
};

class in_buf1_t : public in_t {
	virtual void __noreturn unexpected_end() const {
		throw exception_log_t(log::error, "unexpected end of in_segment_t");
	}

	virtual bool do_expand() {
		if(!*strs)
			return false;

		*last = new page_buf_t(off_end, *strs);
		last = &((*last)->next);
		off_end += strs->size();
		++strs;
		return true;
	}

	ref_t<page_t> *last;
	string_t const *strs;

public:
	inline in_buf1_t(string_t const *_strs) : in_t(), last(&list), strs(_strs) { }

};

static void test1() {
	{
		in_buf0_t in(inbuf1);
		print(in);
	}
	{
		in_buf1_t in(inbuf1);
		print(in);
	}
}

static string_t const inbuf2[] = {
	STRING("0000000 1111111 2222"),
	STRING("222 3333333 4444444 "),
	STRING("5555555 6666666 77777"),
	STRING("77 8888888 9999999"),
	string_t::empty
};

static void test2_2(in_t &in) {
	bool push = true;
	in_t::ptr_t ptr = in;

	in_segment_list_t in_segment_list;

	in_t::ptr_t p = ptr;

	while(p) {
		char c = *p;
		++p;
		if(c == ' ') {
			if(push)
				in_segment_list.append(in_segment_t(ptr, p - ptr));

			push = !push;
			ptr = p;
		}
	}

	if(push && p > ptr)
		in_segment_list.append(in_segment_t(ptr, p - ptr));

	print(in_segment_list);
}

static void test2() {
	in_buf1_t in(inbuf2);

	in_t::ptr_t ptr = in;

	ptr += 8;

	in_t::ptr_t p = ptr;
	while(p) ++p;

	in_segment_t segment(ptr, p - ptr - 8);

	print(in);
	print(segment);

	test2_2(in);
	test2_2(segment);
}

extern "C" int main() {
	test1();
	test2();
}
