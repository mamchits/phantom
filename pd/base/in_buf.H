// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/in.H>
#include <pd/base/log.H>

#pragma GCC visibility push(default)

namespace pd {

class in_buf_t : public in_t {
	virtual bool do_expand();
	virtual void __noreturn unexpected_end() const;

	virtual size_t readv(iovec *iov, size_t cnt) = 0;

	class __hidden page_t;

	size_t page_size;
	log::level_t log_level;
	ref_t<in_t::page_t> *last;
	page_t *last_page;
	page_t *reserved_page;

protected:
	inline in_buf_t(size_t _page_size, log::level_t _log_level) :
		page_size(_page_size), log_level(_log_level), last(&list),
		last_page(NULL), reserved_page(NULL) { }

	~in_buf_t() throw();

protected:
	inline log::level_t get_log_level() const { return log_level; }

public:
	bool truncate(ptr_t &ptr);

private: // don't use
	in_buf_t(in_buf_t const &);
	in_buf_t &operator=(in_buf_t const &);
};

} // namespace pd

#pragma GCC visibility pop
