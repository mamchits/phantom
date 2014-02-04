// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "log_file.H"
#include "config_helper.H"
#include "exception.H"
#include "fd_guard.H"
#include "stat_items.H"
#include "stat.H"

#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/file.h>

namespace pd {

namespace log_file {

class id_t {
	dev_t dev;
	ino_t ino;

public:
	inline id_t() : dev(0), ino(0) { }

	inline id_t(struct stat const &st) : dev(st.st_dev), ino(st.st_ino) { }

	inline id_t(int fd, bool &used) : dev(0), ino(0) {
		struct stat st;
		if(::fstat(fd, &st) == 0) {
			dev = st.st_dev;
			ino = st.st_ino;
			used = (st.st_size != 0);
		}
	}

	inline operator bool() const { return ino != 0; }

	friend inline bool operator==(id_t const &id1, id_t const &id2) {
		return id1.dev == id2.dev && id1.ino == id2.ino;
	}

	friend inline bool operator!=(id_t const &id1, id_t const &id2) {
		return !(id1 == id2);
	}
};

struct file_t {
	unsigned int ref_cnt;
	int fd;
	bool used;
	id_t id;

	file_t(char const *name,  string_t const &header);
	~file_t() throw();
};

file_t::file_t(char const *name, string_t const &header) :
	ref_cnt(0), fd(-1), used(false), id() {

	if(config::check)
		return;

	fd = ::open(name, O_WRONLY | O_APPEND | O_CREAT, 0644);
	if(fd < 0)
		throw exception_sys_t(log::error, errno, "open (%s): %m", name);

	fd_guard_t guard(fd);

	id = id_t(fd, used);
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

file_t::~file_t() throw() {
	if(config::check)
		return;

	if(::close(fd) < 0)
		log_error("log_file_handler_t::~log_file_handler_t, close: %m");
}

struct handler_t {
	string_t const filename_z;
	string_t const header;

	spinlock_t spinlock;
	file_t *current, *old;

	typedef stat::count_t records_t;
	typedef stat::count_t ocount_t;

	typedef stat::items_t<
		records_t,
		ocount_t
	> stat_base_t;

	struct stat_t : stat_base_t {
		inline stat_t() throw() : stat_base_t(
			STRING("records"),
			STRING("out")
		) { }

		inline ~stat_t() throw() { }

		inline records_t &records() { return item<0>(); }
		inline ocount_t &ocount() { return item<1>(); }
	};

	stat_t stat;

	inline int acquire(bool &used) throw() {
		++stat.records();

		spinlock_guard_t guard(spinlock);
		++current->ref_cnt;
		used = current->used;
		current->used = true;
		return current->fd;
	}

	inline void release(int fd) throw() {
		spinlock_guard_t guard(spinlock);
		if(current->fd == fd)
			--current->ref_cnt;
		else if(old && old->fd == fd)
			--old->ref_cnt;
	}

	void check();

	inline handler_t(string_t const &filename, string_t const &_header) :
		filename_z(string_t::ctor_t(filename.size() + 1)(filename)('\0')),
		header(_header), current(NULL), old(NULL), stat() {

		current = new file_t(filename_z.ptr(), header);
	}

	inline ~handler_t() throw() {
		if(old) delete old;
		delete current;
	}
};

void handler_t::check() {
	file_t *tmp = NULL;

	bool skip_file_check = false;

	{
		spinlock_guard_t guard(spinlock);
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
		if(id_t(st) == current->id) // Don't need lock
			return;
	}
	else {
		if(errno != ENOENT)
			log_error("stat: (%s) %m", filename_z.ptr());
	}

	tmp = new file_t(filename_z.ptr(), header);

	{
		spinlock_guard_t guard(spinlock);
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

} // namespace log_file

log_file_t::guard_t::guard_t(log_file_t const &log_file) throw() :
	handler(log_file.handler), used(false),
	fd(handler.acquire(used)), ocount(handler.stat.ocount()) { }

log_file_t::guard_t::~guard_t() throw() { handler.release(fd); }

log_file_t::log_file_t(string_t const &filename, string_t const &_header) :
	handler(*new handler_t(filename, _header)) { }

log_file_t::~log_file_t() throw() { delete &handler; }

void log_file_t::_init() { handler.stat.init(); }

void log_file_t::_stat_print() const { handler.stat.print(); }

void log_file_t::check() const { handler.check(); }

ssize_t log_file_t::writev(iovec const *iov, size_t count) const throw() {
	guard_t guard(*this);

	ssize_t res = ::writev(guard.fd, iov, count);

	if(res > 0)
		guard.ocount += res;

	return res;
}

} // namespace pd
