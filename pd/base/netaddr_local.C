// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "netaddr_local.H"

namespace pd {

template<>
void out_t::helper_t<netaddr_local_t>::print(
	out_t &out, netaddr_local_t const &netaddr, char const *
) {
	if(netaddr) {
		str_t::ptr_t ptr = netaddr.path();

		while(ptr) out(*(ptr++) ?: '@');
	}
	else
		out(CSTR("<unset>"));
}

netaddr_local_t::netaddr_local_t(str_t const &path) {
	sa_len = ({
		out_t out(sun.sun_path, sizeof(sun.sun_path));
		out(path)('\0');
		out.used();
	}) + (sizeof(sun) - sizeof(sun.sun_path));

	sun.sun_family = AF_LOCAL;
}

netaddr_local_t::netaddr_local_t(char const *sys_path_z) {
	sa_len = ({
		out_t out(sun.sun_path, sizeof(sun.sun_path));
		out(str_t(sys_path_z, strlen(sys_path_z)))('\0');
		out.used();
	}) + (sizeof(sun) - sizeof(sun.sun_path));

	sun.sun_family = AF_LOCAL;
}

void netaddr_local_t::print(out_t &out, char const *) const {
	out.print(*this);
}

size_t netaddr_local_t::print_len() const {
	return path().size();
}

netaddr_local_t::~netaddr_local_t() throw() { }

} // namespace pd
