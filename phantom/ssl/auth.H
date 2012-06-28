// This file is part of the phantom::ssl module.
// Copyright (C) 2011, 2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011, 2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/ssl/ssl.H>

#pragma GCC visibility push(default)

namespace phantom { namespace ssl {

class auth_t : public ssl_auth_t {
public:
	struct config_t;

	auth_t(string_t const &name, config_t const &config);
	~auth_t() throw();

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

}} // namespace phantom::ssl

#pragma GCC visibility pop
