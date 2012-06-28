// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "in_buf.H"
#include "exception.H"

namespace pd {

class in_buf_t::page_t : public in_t::page_t {
	size_t off_begin;
	size_t off_end;
	char data[0];

	virtual bool chunk(size_t off, str_t &str) const;
	virtual unsigned int depth() const;
	virtual bool optimize(in_segment_t &segment) const;

	inline page_t(size_t _off_begin, size_t size) throw() :
		in_t::page_t(), off_begin(_off_begin), off_end(_off_begin + size) { }

	virtual ~page_t() throw();

	inline void *operator new(size_t size, size_t body_size) {
		return ::operator new(size + body_size);
	}

	friend class in_buf_t;
};

bool in_buf_t::page_t::chunk(size_t off, str_t &str) const {
	if(off < off_begin || off >= off_end)
		return false;

	str = str_t(data + (off - off_begin), off_end - off);

	return true;
}

unsigned int in_buf_t::page_t::depth() const { return 0; }

bool in_buf_t::page_t::optimize(in_segment_t &) const { return false; }

in_buf_t::page_t::~page_t() throw() { }

bool in_buf_t::do_expand() {
	if(!last_page) {
		*last = last_page = new(page_size) page_t(off_end, page_size);
		last = &last_page->next;
	}
	else if(!reserved_page && last_page->off_begin < off_end) {
		reserved_page = new(page_size) page_t(last_page->off_end, page_size);
	}

	iovec invec[2];

	invec[0].iov_base = last_page->data + (off_end - last_page->off_begin);
	invec[0].iov_len = last_page->off_end - off_end;

	if(reserved_page) {
		invec[1].iov_base = reserved_page->data;
		invec[1].iov_len = page_size;
	}

	size_t res = readv(invec, reserved_page ? 2 : 1);

	off_end += res;

	if(off_end >= last_page->off_end) {
		if((last_page = reserved_page)) {
			*last = last_page;
			last = &last_page->next;
			reserved_page = NULL;
		}
	}

	return res > 0;
}

bool in_buf_t::truncate(ptr_t &ptr) {
	assert(ptr.in == this);

	size_t off = ptr.off();
	assert(off >= off_begin);

	if(!expand(off))
		unexpected_end();

	if(off_end > off) {
		while(true) {
			assert(list != NULL);
			str_t str;
			if(list->chunk(off, str))
				break;

			list = list->next;
		}
	}
	else {
		last_page = NULL;
		list = NULL;
		last = &list;
	}

	if(reserved_page) {
		delete reserved_page;
		reserved_page = NULL;
	}

	off_begin = off;

	ptr = ptr_t(*this);

	return off_begin < off_end;
}

in_buf_t::~in_buf_t() throw() {
	if(reserved_page)
		delete reserved_page;
}

void __noreturn in_buf_t::unexpected_end() const {
	throw exception_log_t(log_level, "unexpected end of input");
}

} // namespace pd
