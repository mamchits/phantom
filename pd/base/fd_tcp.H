// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/fd.H>

#pragma GCC visibility push(default)

namespace pd {

class fd_ctl_tcp_t : public fd_ctl_t {
	bool cork;

public:
	virtual void operator()(int fd, int i) const;

	inline fd_ctl_tcp_t(bool _cork) throw() : cork(_cork) { }
	inline ~fd_ctl_tcp_t() throw() { }
};

} // namespace pd

#pragma GCC visibility pop
