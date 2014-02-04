// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "string_file.H"
#include "fd_guard.H"
#include "exception.H"

#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

namespace pd {

string_t string_file(char const *sys_filename) {
	int fd = ::open(sys_filename, O_RDONLY, 0);
	if(fd < 0)
		throw exception_sys_t(log::error, errno, "open (%s): %m", sys_filename);

	fd_guard_t guard(fd);

	struct stat st;
	if(fstat(fd, &st) < 0)
		throw exception_sys_t(log::error, errno, "stat (%s): %m", sys_filename);

	size_t size = st.st_size;
	if(size) {
		off_t off = 0;
		string_t::ctor_t ctor(size);
		ctor.sendfile(fd, off, size);
		return ctor;
	}

	return string_t::empty;
}

string_t string_file(str_t const &filename) {
	MKCSTR(_filename, filename);

	return string_file(_filename);
}

string_t string_file(string_t const &filename) {
	MKCSTR(_filename, filename);

	return string_file(_filename);
}

} // namespace pd
