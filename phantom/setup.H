// This file is part of the phantom program.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#pragma GCC visibility push(default)

namespace phantom {

class setup_t {
protected:
	inline setup_t() throw() { }
	inline ~setup_t() throw() { }

private: // don't use
	setup_t(setup_t const &);
	setup_t &operator=(setup_t const &);
};

#pragma GCC visibility pop

} // namespace phantom
