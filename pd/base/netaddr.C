// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "netaddr.H"

namespace pd {

netaddr_t::~netaddr_t() throw() { }

template<>
void out_t::helper_t<netaddr_t>::print(
	out_t &out, netaddr_t const &netaddr, char const *fmt
) {
	netaddr.print(out, fmt);
}

} // namespace pd
