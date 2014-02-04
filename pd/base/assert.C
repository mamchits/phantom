// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "assert.H"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

namespace pd {

void __noreturn __abort(
	char const *tag, char const *msg, char const *file, unsigned int line
) throw() {
	char buf[128];

	snprintf(
		buf, sizeof(buf), "%s: '%s' at file: %s, line: %u",
		tag, msg, file, line
	);

	buf[sizeof(buf) - 2] = '\0';
	size_t l = strlen(buf);
	buf[l++] = '\n'; buf[l] = '\0';

	ssize_t __attribute__((unused)) n = write(2, buf, l);
	abort();
}

} // namespace pd
