// This file is part of the phantom::io_benchmark::method_stream::transport_ssl module.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../transport.H"
#include "../../../module.H"
#include "../../../ssl/auth.H"

#include <pd/ssl/bq_conn_ssl.H>

#include <pd/bq/bq_util.H>
#include <pd/bq/bq_spec.H>

#include <pd/base/config.H>
#include <pd/base/fd.H>
#include <pd/base/log.H>

#include <unistd.h>

namespace phantom { namespace io_benchmark { namespace method_stream {

MODULE(io_benchmark_method_stream_transport_ssl);

class transport_ssl_t : public transport_t {
public:
	typedef ssl::auth_t auth_t;

private:
	class conn_ssl_t : public conn_t {
		bq_conn_ssl_t bq_conn_ssl;

		virtual operator bq_conn_t &();

	public:
		inline conn_ssl_t(
			int fd, ssl_ctx_t const &ctx, interval_t _timeout, fd_ctl_t const *_ctl
		) : conn_t(fd), bq_conn_ssl(fd, ctx, _timeout, _ctl) { }

		virtual ~conn_ssl_t() throw();
	};

	ssl_ctx_t ctx;
	interval_t timeout;

public:
	struct config_t {
		config_binding_type_ref(auth_t);
		config::objptr_t<auth_t> auth;
		string_t ciphers;
		interval_t timeout;

		inline config_t() : auth(), ciphers(), timeout(interval::second) { }

		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &) const { }
	};

	inline transport_ssl_t(string_t const &, config_t const &config) :
		transport_t(), ctx(ssl_ctx_t::client, config.auth, config.ciphers),
		timeout(config.timeout) { }

	inline ~transport_ssl_t() throw() { }

	virtual conn_t *new_connect(int fd, fd_ctl_t const *_ctl) const;
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

conn_t *transport_ssl_t::new_connect(int fd, fd_ctl_t const *_ctl) const {
	return new conn_ssl_t(fd, ctx, timeout, _ctl);
}

transport_ssl_t::conn_ssl_t::operator bq_conn_t &() {
	return bq_conn_ssl;
}

transport_ssl_t::conn_ssl_t::~conn_ssl_t() throw() { }

}}} // namespace phantom::io_benchmark::method_stream
