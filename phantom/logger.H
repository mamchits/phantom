// This file is part of the phantom program.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/log.H>

#pragma GCC visibility push(default)

namespace phantom {

class logger_t : public log::backend_t {
protected:
	inline logger_t() throw() : log::backend_t() { }
	inline ~logger_t() throw() { }

protected:
	void *operator new(size_t);
	void operator delete(void *);
};

} // namespace phantom

#pragma GCC visibility pop
