// This file is part of the phantom::io_stream module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/netaddr.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_stream {

class acl_t {
public:
	enum policy_t { unset = 0, allow, deny };
	virtual bool check(netaddr_t const &netaddr) const throw() = 0;

protected:
	inline acl_t() throw() { }
	inline ~acl_t() throw() { }

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

}} // namespace phantom::io_stream

#pragma GCC visibility pop
