// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#pragma GCC visibility push(default)

#include <pd/base/out.H>

#include <sys/socket.h>

namespace pd {

struct netaddr_t {
	socklen_t sa_len;
	sockaddr sa[0];

	inline operator bool() const throw() { return sa->sa_family; }
	virtual void print(out_t &out, char const *fmt = NULL) const = 0;
	virtual size_t print_len() const = 0;
	virtual ~netaddr_t() throw();
protected:
	inline netaddr_t() throw() : sa_len(0) { }
	inline netaddr_t(socklen_t _sa_len) throw() : sa_len(_sa_len) {
		memset(sa, 0, sa_len);
	}
};

} // namespace pd

#pragma GCC visibility pop
