// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/defs.H>

#pragma GCC visibility push(default)

#include <stddef.h>

namespace phantom {

class arena_t {
	class __hidden impl_t;
	impl_t *impl;

public:
	arena_t() throw();
	~arena_t() throw();

	void *alloc(size_t size) throw();
	void *realloc(void *optr, size_t size) throw();
	void free(void *ptr) throw();

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

} // namespace phantom

#pragma GCC visibility pop
