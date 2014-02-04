// This file is part of the phantom::io_stream module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/shared.H>

#include <pd/base/netaddr.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_stream {

class proto_t : public shared_t {
public:
	virtual bool request_proc(
		in_t::ptr_t &ptr, out_t &out,
		netaddr_t const &local_addr, netaddr_t const &remote_addr
	) const = 0;

protected:
	inline proto_t(string_t const &name) throw() : shared_t(name) { }
	inline ~proto_t() throw() { }
};

}} // namespace phantom::io_stream

#pragma GCC visibility pop
