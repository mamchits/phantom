// This file is part of the phantom::ssl module.
// Copyright (C) 2011, 2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011, 2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "auth.H"

#include "../setup.H"

#include <pd/base/config.H>

namespace phantom {

class setup_ssl_t : public setup_t {
public:
	typedef ssl::auth_t auth_t;

	struct config_t {
		config_binding_type_ref(auth_t);

		inline config_t() throw() { }

		inline void check(in_t::ptr_t const &) const { }

		inline ~config_t() throw() { }
	};

	inline setup_ssl_t(string_t const &, config_t const &) { }
	inline ~setup_ssl_t() throw() { }
};

namespace setup_ssl {
config_binding_sname(setup_ssl_t);
config_binding_type(setup_ssl_t, auth_t);
config_binding_ctor(setup_t, setup_ssl_t);
}

} // namespace phantom
