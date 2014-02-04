// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "in.H"
#include "exception.H"

namespace pd {

in_t::page_t::~page_t() throw() { }

bool in_t::ptr_t::update() {
	size_t _off = off();

	assert(_off >= in->off_begin);

	if(!in->expand(_off + 1))
		return false;

	if(!page)
		page = in->list;

	for(; page; page = page->next) {
		str_t str;
		if(page->chunk(_off, str)) {
			str.truncate(in->off_end - _off);

			assert(str.size() > 0);

			ptr = str.ptr();
			bound = ptr + str.size();
			off_bound = _off + (bound - ptr);

			return true;
		}
	}

	fatal("in_t internal error");
}

bool in_t::ptr_t::scan(char const *acc, size_t acc_len, size_t &limit) {
	size_t _limit = limit;
	ptr_t p0 = *this;

	while(_limit) {
		if(!p0)
			return false;

		size_t _len = min(_limit, (size_t)(p0.bound - p0.ptr));

		size_t __len = _len;

		for(size_t i = 0; i < acc_len; ++i) {
			char const *_p = (char const *)memchr(p0.ptr, acc[i], __len);
			if(_p) minimize(__len, _p - p0.ptr);
		}

		p0 += __len;
		_limit -= __len;

		if(__len < _len) {
			*this = p0;
			limit = _limit;
			return true;
		}
	}

	return false;
}

class in_segment_list_t::page_t : public in_t::page_t {
	size_t off_base;
	in_segment_t segment;

	virtual bool chunk(size_t off, str_t &str) const;
	virtual unsigned int depth() const;
	virtual bool optimize(in_segment_t &segment) const;

public:
	inline page_t(size_t _off_base, in_t::ptr_t &ptr, size_t len) :
		off_base(_off_base), segment(ptr, len) { }

	inline page_t(size_t _off_base, in_segment_t const &_segment) :
		off_base(_off_base), segment(_segment) { }
};

bool in_segment_list_t::page_t::chunk(size_t off, str_t &str) const {
	assert(off >= off_base);
	return segment.__chunk(off - off_base, str);
}

unsigned int in_segment_list_t::page_t::depth() const {
	return segment.depth() + 1;
}

bool in_segment_list_t::page_t::optimize(in_segment_t &_segment) const {
	assert(_segment.off() >= off_base);

	size_t _off = _segment.off() - off_base;
	size_t _size = _segment.size();

	if(_off + _size <= segment.size()) {
		in_t::ptr_t ptr = segment;
		ptr += _off;
		_segment = in_segment_t(ptr, _size);

		return true;
	}

	return false;
}

bool in_segment_t::do_expand() { return false; }

void __noreturn in_segment_t::unexpected_end() const {
	throw exception_log_t(log::error | log::trace, "unexpected end of in_segment_t");
}

void in_segment_list_t::append(in_segment_t const &_segment) {
	if(_segment) {
		page_t *page = new page_t(off_end, _segment);
		*last = page;
		last = &page->next;

		off_end += _segment.size();
	}
}

} // namespace pd
