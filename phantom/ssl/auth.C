// This file is part of the phantom::ssl module.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "auth.H"

#include "../module.H"

#include <pd/base/config.H>
#include <pd/base/config_enum.H>

namespace phantom { namespace ssl {

MODULE(ssl);

struct auth_t::config_t {
	string_t key, cert;

	inline config_t() : key(), cert() { }

	inline void check(in_t::ptr_t const &) const { }

	inline ~config_t() throw() { }
};

namespace auth_server {
config_binding_sname(auth_t);
config_binding_value(auth_t, key);
config_binding_value(auth_t, cert);
config_binding_ctor_(auth_t);
}

auth_t::auth_t(string_t const &, config_t const &config) :
	ssl_auth_t(config.key, config.cert) { }

auth_t::~auth_t() throw() { }

}} // namespace phantom::ssl
