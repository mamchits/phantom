// This file is part of the pd::ssl library.
// Copyright (C) 2011, 2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011, 2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/string.H>

#pragma GCC visibility push(default)

namespace pd {

class ssl_auth_t {
	string_t key, cert;

public:
	ssl_auth_t(string_t const &key_fname, string_t const &cert_fname);

	~ssl_auth_t() throw();

	friend class ssl_ctx_t;

private: // don't use
	ssl_auth_t(ssl_auth_t const &);
	ssl_auth_t &operator=(ssl_auth_t const &);
};

class ssl_ctx_t {
public:
	void *internal;
	enum mode_t { client, server };

	ssl_ctx_t(mode_t _mode, ssl_auth_t const *auth, string_t const &ciphers);

	~ssl_ctx_t() throw();

private: // don't use
	ssl_ctx_t(ssl_ctx_t const &);
	ssl_ctx_t &operator=(ssl_ctx_t const &);
};

} // namespace pd

#pragma GCC visibility pop
