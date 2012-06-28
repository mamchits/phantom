// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "netaddr_ipv4.H"

namespace pd {

template<>
void out_t::helper_t<netaddr_ipv4_t>::print(
	out_t &out, netaddr_ipv4_t const &netaddr, char const *
) {
	if(netaddr)
		out.print(netaddr.address())(':').print(netaddr.port());
	else
		out(CSTR("<unset>"));
}

void netaddr_ipv4_t::print(out_t &out, char const *) const {
	out.print(*this);
}

size_t netaddr_ipv4_t::print_len() const {
	return 3 + 1 + 3 + 1 + 3 + 1 + 3 + 1 + 5;
}

netaddr_ipv4_t::~netaddr_ipv4_t() throw() { }

} // namespace pd
