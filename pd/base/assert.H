// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/defs.H>

#pragma GCC visibility push(default)

namespace pd {

void __noreturn __abort(
	char const *tag, char const *msg, char const *file, unsigned int line
) throw();

#define assert(expr) \
	if(__builtin_expect(!(expr), 0)) \
		::pd::__abort("Assertion failed", #expr, __FILE__, __LINE__)

#define fatal(msg) \
	::pd::__abort("Fatal", msg, __FILE__, __LINE__)

} // namespace pd

#pragma GCC visibility pop
