// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "log_file_handler.H"
#include "config_helper.H"
#include "exception.H"
#include "fd_guard.H"

#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/file.h>

namespace pd {

class file_id_t {
	dev_t dev;
	ino_t ino;

public:
	inline file_id_t() : dev(0), ino(0) { }

	inline file_id_t(struct stat const &st) : dev(st.st_dev), ino(st.st_ino) { }

	inline file_id_t(int fd) : dev(0), ino(0) {
		struct stat st;
		if(::fstat(fd, &st) == 0) {
			dev = st.st_dev;
			ino = st.st_ino;
		}
	}

	inline operator bool() const { return ino != 0; }

	friend inline bool operator==(file_id_t const &id1, file_id_t const &id2) {
		return id1.dev == id2.dev && id1.ino == id2.ino;
	}

	friend inline bool operator!=(file_id_t const &id1, file_id_t const &id2) {
		return !(id1 == id2);
	}
};

class log_file_handler_t::file_t {
	unsigned int ref_cnt;
	int fd;
	file_id_t id;

	file_t(char const *name,  string_t const &header);
	~file_t() throw();

	friend class log_file_handler_t;
};

log_file_handler_t::file_t::file_t(char const *name, string_t const &header) :
	ref_cnt(0), fd(-1), id() {

	if(config::check)
		return;

	fd = ::open(name, O_WRONLY | O_APPEND | O_CREAT, 0644);
	if(fd < 0)
		throw exception_sys_t(log::error, errno, "open (%s): %m", name);

	fd_guard_t guard(fd);

	id = file_id_t(fd);
	if(!id)
		throw exception_sys_t(log::error, errno, "fstat (%s): %m", name);

	if(::flock(fd, LOCK_SH | LOCK_NB) < 0)
		throw exception_sys_t(log::error, errno, "flock (%s): %m", name);

	if(header) {
		if(::write(fd, header.ptr(), header.size()) < 0)
			throw exception_sys_t(log::error, errno, "write (%s): %m", name);
	}

	guard.relax();
}

log_file_handler_t::file_t::~file_t() throw() {
	if(config::check)
		return;

	if(::close(fd) < 0)
		log_error("log_file_handler_t::~log_file_handler_t, close: %m");
}

log_file_handler_t::log_file_handler_t(
	string_t const &filename, string_t const &_header
) :
	spinlock(),
	filename_z(string_t::ctor_t(filename.size() + 1)(filename)('\0')),
	header(_header), current(NULL), old(NULL) {

	current = new file_t(filename_z.ptr(), header);
}

log_file_handler_t::~log_file_handler_t() throw() {
	if(old) delete old;
	delete current;
}

void log_file_handler_t::check() {
	file_t *tmp = NULL;

	bool skip_file_check = false;

	{
		thr::spinlock_guard_t guard(spinlock);
		if(old) {
			if(old->ref_cnt)
				skip_file_check = true;
			else {
				tmp = old;
				old = NULL;
			}
		}
	}

	if(tmp) { delete tmp; tmp = NULL; }
	if(skip_file_check) return;

	struct stat st;
	if(::stat(filename_z.ptr(), &st) == 0) {
		if(file_id_t(st) == current->id) // Don't need lock
			return;
	}
	else {
		if(errno != ENOENT)
			log_error("stat: (%s) %m", filename_z.ptr());
	}

	tmp = new file_t(filename_z.ptr(), header);

	{
		thr::spinlock_guard_t guard(spinlock);
		if(!old) {
			old = current;
			current = tmp;
			tmp = NULL;
		}
		if(!old->ref_cnt) {
			tmp = old;
			old = NULL;
		}
	}

	if(tmp) { delete tmp; tmp = NULL; }
}

int log_file_handler_t::acquire() const throw() {
	thr::spinlock_guard_t guard(spinlock);
	++current->ref_cnt;
	return current->fd;
}

void log_file_handler_t::release(int fd) const throw() {
	thr::spinlock_guard_t guard(spinlock);
	if(current->fd == fd)
		--current->ref_cnt;
	else if(old && old->fd == fd)
		--old->ref_cnt;
}

ssize_t log_file_handler_t::writev(
	iovec const *iov, size_t count
) const throw() {
	int fd = acquire();
	ssize_t res = ::writev(fd, iov, count);
	release(fd);
	return res;
}

} // namespace pd
