// This file is part of the phantom::io_stream::transport_ssl module.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../transport.H"
#include "../../module.H"
#include "../../ssl/auth.H"

#include <pd/ssl/bq_conn_ssl.H>

#include <pd/base/config.H>
#include <pd/base/fd.H>
#include <pd/base/exception.H>

namespace phantom { namespace io_stream {

MODULE(io_stream_transport_ssl);

class transport_ssl_t : public transport_t {
public:
	typedef ssl::auth_t auth_t;

private:
	ssl_ctx_t ctx;
	interval_t timeout;

	virtual bq_conn_t *new_connect(
		int fd, fd_ctl_t const *_ctl, log::level_t remote_errors
	) const;

public:
	struct config_t {
		config_binding_type_ref(auth_t);
		config::objptr_t<auth_t> auth;
		string_t ciphers;
		interval_t timeout;

		inline config_t() throw() : auth(), ciphers(), timeout(interval::second) { }
		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &ptr) const {
			if(!auth)
				config::error(ptr, "auth is required");
		}
	};

	inline transport_ssl_t(string_t const &, config_t const &config) throw() :
		ctx(ssl_ctx_t::server, config.auth, config.ciphers),
		timeout(config.timeout) { }

	inline ~transport_ssl_t() throw() { }
};

namespace transport_ssl {
config_binding_sname(transport_ssl_t);
config_binding_type(transport_ssl_t, auth_t);
config_binding_value(transport_ssl_t, auth);
config_binding_value(transport_ssl_t, ciphers);
config_binding_value(transport_ssl_t, timeout);
config_binding_cast(transport_ssl_t, transport_t);
config_binding_ctor(transport_t, transport_ssl_t);
}

bq_conn_t *transport_ssl_t::new_connect(
	int fd, fd_ctl_t const *_ctl, log::level_t remote_errors
) const {
	return new bq_conn_ssl_t(fd, ctx, timeout, _ctl, remote_errors);
}

}} // namespace phantom::io_stream
