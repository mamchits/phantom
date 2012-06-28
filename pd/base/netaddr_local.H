// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/netaddr.H>
#include <pd/base/string.H>
#include <pd/base/assert.H>

#include <sys/un.h>

#pragma GCC visibility push(default)

namespace pd {

class netaddr_local_t : public netaddr_t {
	sockaddr_un sun;

public:
	inline netaddr_local_t() throw() : netaddr_t(sizeof(sun)) {
		assert((sockaddr *)&sun == sa);
		sun.sun_family = AF_UNSPEC;
	}

	netaddr_local_t(str_t const &path);
	netaddr_local_t(char const *sys_path_z);

	virtual void print(out_t &out, char const *fmt = NULL) const;
	virtual size_t print_len() const;
	virtual ~netaddr_local_t() throw();

	inline str_t path() const throw() {
		return str_t(sun.sun_path, sa_len - (sizeof(sun) - sizeof(sun.sun_path)));
	}

	static inline size_t max_len() {
		return sizeof(((sockaddr_un *)NULL)->sun_path);
	}
};

} // namespace pd

#pragma GCC visibility pop
