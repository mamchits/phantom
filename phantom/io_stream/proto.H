// This file is part of the phantom::io_stream module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/netaddr.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_stream {

class proto_t {
public:
	virtual bool request_proc(
		in_t::ptr_t &ptr, out_t &out,
		netaddr_t const &local_addr, netaddr_t const &remote_addr
	) = 0;

	virtual void stat(out_t &out, bool clear) = 0;

protected:
	inline proto_t() throw() { }
	inline ~proto_t() throw() { }

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

}} // namespace phantom::io_stream

#pragma GCC visibility pop
