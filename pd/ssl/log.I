// This file is part of the pd::ssl library.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/log.H>

#include <openssl/err.h>

namespace pd {

static inline void log_openssl_error(log::level_t log_level) throw() {
	unsigned long l;

	// FIXME: Use log::aux_t here

	while((l = ERR_get_error())) {
		char buf[128];
		ERR_error_string_n(l, buf, sizeof(buf));
		buf[sizeof(buf) - 1] = '\0';

		log_put(log_level, NULL, "SSL %s", buf);
	}
}

} // namespace pd
