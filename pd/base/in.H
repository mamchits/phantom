// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/assert.H>
#include <pd/base/ref.H>
#include <pd/base/str.H>

#include <sys/uio.h>

#pragma GCC visibility push(default)

namespace pd {

class in_segment_t;

class in_t {
	virtual bool do_expand() = 0;
	virtual void __noreturn unexpected_end() const = 0;

protected:
	inline bool expand(size_t off) const {
		while(off > off_end) {
			if(!((in_t *)this)->do_expand()) // FIXME: dirty hack
				return false;
		}
		return true;
	}

public:
	class page_t : public ref_count_atomic_t {
	protected:
		inline page_t() throw() : next() { }

		virtual ~page_t() throw();

	public:
		ref_t<page_t> next;

		virtual bool chunk(size_t off, str_t &str) const = 0;
		virtual unsigned int depth() const = 0;
		virtual bool optimize(in_segment_t &segment) const = 0;

		friend class ref_t<page_t>;
	};

protected:
	size_t off_begin;
	size_t off_end;
	ref_t<page_t> list;

	inline in_t() : off_begin(0), off_end(0), list() { }

	inline ~in_t() throw() { }

public:

	void clear() {
		off_begin = off_end = 0; list = NULL;
	}

	class ptr_t;

	typedef void (*error_handler_t)(ptr_t const &ptr, char const *msg);

	template<typename x_t>
	struct helper_t {
		static bool parse(ptr_t &ptr, x_t &x, char const *fmt, error_handler_t handler);
	};

	class ptr_t : public ptr_ops_t<ptr_t> {
		char const *ptr, *bound;
		size_t off_bound;
		page_t *page;
		in_t const *in;

		operator int() const; // to prevent incorrect conversions to bool

		inline size_t off() const { return off_bound - (bound - ptr); }

		bool update();

	public:
		inline ptr_t(in_t const &_in) :
			ptr(NULL), bound(NULL), off_bound(_in.off_begin),
			page(_in.list), in(&_in) { }

		inline operator bool() {
			return (ptr < bound) || update();
		}

		inline char operator*() {
			if(!*this)
				in->unexpected_end();

			return *ptr;
		}

		inline str_t __chunk() {
			if(!*this)
				in->unexpected_end();

			return str_t(ptr, bound - ptr);
		}

		inline ptr_t &operator+=(size_t len) throw() {
			ptr += len;
			return *this;
		}

		inline void verify(size_t len) {
			if(!*this || !in->expand(off() + len))
				in->unexpected_end();
		}

		inline ptr_t &seek_end() {
			if(off() < in->off_end)
				ptr = bound + (in->off_end - off_bound);

			return *this;
		}

		inline size_t pending() const {
			size_t _off = off();
			return (_off < in->off_end) ? in->off_end - _off : 0;
		}

		friend inline ptrdiff_t operator-(ptr_t const &p1, ptr_t const &p2) throw() {
			assert(p1.in == p2.in);
			return p1.off() - p2.off();
		}

		friend inline bool operator==(ptr_t const &p1, ptr_t const &p2) throw() {
			assert(p1.in == p2.in);
			return p1.off() == p2.off();
		}

		friend inline bool operator<(ptr_t const &p1, ptr_t const &p2) throw() {
			assert(p1.in == p2.in);
			return p1.off() < p2.off();
		}

		template<typename type_t>
		inline bool match(str_t const &_str) {
			ptr_t p0 = *this;
			str_t::ptr_t p1 = _str;

			while(p1) {
				if(!p0)
					return false;

				size_t _len = min(p1.size(), (size_t)(p0.bound - p0.ptr));

				if(!type_t::cmp(p0.ptr, p1(), _len))
					return false;

				p0 += _len; p1 += _len;
			}

			*this = p0;

			return true;
		}

		template<typename type_t>
		inline bool match(char c) {
			if(*this && type_t::map(*ptr) == type_t::map(c)) {
				++ptr;
				return true;
			}
			return false;
		}

		bool scan(char const *acc, size_t acc_len, size_t &limit);

		template<typename x_t>
		inline bool parse(x_t &x, char const *fmt, error_handler_t handler = NULL) {
			return helper_t<x_t>::parse(*this, x, fmt, handler);
		}

		template<typename x_t>
		inline bool parse(x_t &x, error_handler_t handler = NULL) {
			return helper_t<x_t>::parse(*this, x, NULL, handler);
		}

		friend class in_segment_t;
		friend class in_buf_t;
	};

	friend class ptr_t;

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

class in_segment_t : public in_t {
	virtual bool do_expand();
	virtual void __noreturn unexpected_end() const;

	operator int() const; // to prevent incorrect conversions to bool

public:
	inline in_segment_t() : in_t() { }

	inline in_segment_t(ptr_t &ptr, size_t len) : in_t() {
		if(len) {
			ptr.verify(len);
			off_begin = ptr.off();
			off_end = off_begin + len;
			list = ptr.page;

			while(list->optimize(*this));
		}
	}

	inline in_segment_t(in_segment_t const &_segment) :
		in_t(_segment) {

		if(size())
			while(list->optimize(*this));
	}

	inline ~in_segment_t() throw() { }

protected:
	inline in_segment_t(size_t _off_begin, size_t _off_end, page_t *_page) :
		in_t() {
		off_begin = _off_begin;
		off_end = _off_end;
		list = _page;

		// TODO: verify _page
	}

public:
	inline size_t off() const { return off_begin; }
	inline size_t size() const { return off_end - off_begin; }
	inline operator bool() const { return off_end > off_begin; }
	inline unsigned int depth() const { return (list ? list->depth() : 0); }

	inline size_t fill(iovec *iov) const {
		ptr_t ptr = *this;
		size_t cnt = 0;

		while(ptr) {
			size_t size = ptr.bound - ptr.ptr;

			if(iov) *(iov++) = (iovec){(void *)ptr.ptr, size };
			ptr += size;
			++cnt;
		}

		return cnt;
	}

	template<typename type_t>
	inline uint64_t fnv() const {
		fnv_t h;
		ptr_t ptr = *this;
		while(ptr) h(type_t::map(*(ptr++)));
		return h;
	}

	template<typename type_t>
	inline bool cmp_eq(str_t str) const {
		if(str.size() != size())
			return false;

		ptr_t ptr = *this; return ptr.match<type_t>(str);
	}

	inline bool __chunk(size_t off, str_t &str) const {
		size_t off_real = off_begin + off;

		if(off_real >= off_end)
			return false;

		for(page_t const *page = list; page; page = page->next) {
			if(page->chunk(off_real, str)) {
				str.truncate(off_end - off_real);
				return true;
			}
		}

		fatal("in_segment_t internal error");
	}
};

class in_segment_list_t : public in_segment_t {
	class __hidden page_t;

	ref_t<in_t::page_t> *last;

public:
	inline in_segment_list_t() : in_segment_t(), last(&list) { }

	void append(in_segment_t const &_segment);

	inline void clear() {
		in_segment_t::clear();
		last = &list;
	}

	inline ~in_segment_list_t() throw() { }

private: // don't use
	in_segment_list_t(in_segment_list_t const &);
	in_segment_list_t &operator=(in_segment_list_t const &);
};

} // namespace pd

#pragma GCC visibility pop
