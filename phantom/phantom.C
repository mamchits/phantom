// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "setup.H"
#include "io.H"
#include "stat.H"
#include "logger.H"
#include "module.H"

#include <pd/bq/bq_spec.H>
#include <pd/bq/bq_thr.H>

#include <pd/base/config_enum.H>
#include <pd/base/config_struct.H>
#include <pd/base/config_list.H>
#include <pd/base/string_file.H>
#include <pd/base/out_fd.H>
#include <pd/base/fd_guard.H>
#include <pd/base/time.H>
#include <pd/base/exception.H>
#include <pd/base/trace.H>

#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/signal.h>
#include <sys/stat.h>

using namespace pd;

class phantom_t {
public:
	typedef phantom::obj_t obj_t;
	typedef phantom::scheduler_t scheduler_t;
	typedef phantom::io_t io_t;
	typedef phantom::logger_t logger_t;
	typedef phantom::setup_t setup_t;

	struct config_t {
		config_binding_type_ref(setup_t);
		config_binding_type_ref(scheduler_t);
		config_binding_type_ref(logger_t);
		config_binding_type_ref(io_t);

		config::objptr_t<logger_t> logger;

		inline config_t() : logger() { }
		inline ~config_t() { }

		inline void check(in_t::ptr_t const &) const { }
	};

	static int run(string_t const &name);
	static int check(string_t const &name);
	static void syntax();
};

namespace phantom {
config_binding_sname(phantom_t);
config_binding_type(phantom_t, setup_t);
config_binding_type(phantom_t, scheduler_t);
config_binding_type(phantom_t, logger_t);
config_binding_value(phantom_t, logger);
config_binding_type(phantom_t, io_t);
config_binding_removed(phantom_t, stat, "stat is obsoleted. Use io_monitor_t instead");
}

int phantom_t::run(string_t const &conf_name) {
	config_t config;
	string_t const conf_str = string_file(conf_name.str());

	try {
		in_t::ptr_t ptr = conf_str;
		config::binding_t<config_t>::parse(ptr, config);
		if(ptr)
			config::error(ptr, "unexpected '}'");
	}
	catch(config::exception_t const &ex) {
		config::report_position(conf_name, conf_str, ex.ptr);
		return 1;
	}

	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	sigaddset(&sigset, SIGTERM);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGQUIT);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	log::handler_t handler(STRING("phantom"), config.logger, true);

	struct obj_guard_t {
		inline obj_guard_t() {
			log_info("Start");
			phantom::obj_init();
		}

		inline ~obj_guard_t() {
			bq_thr_t::stop();

			phantom::obj_fini();
			log_info("Exit");
		}
	};

	obj_guard_t obj_guard;

	phantom::obj_exec();

	for(bool work = true; work;) {
		switch(({ int sig; sigwait(&sigset, &sig); sig; })) {
			case SIGQUIT:
			case SIGTERM:
			case SIGINT:
				work = false;
			case SIGALRM:
				;
		}
	}

	return 0;
}

int phantom_t::check(string_t const &conf_name) {
	config::check = true;
	config_t config;
	string_t const conf_str = string_file(conf_name.str());

	try {
		in_t::ptr_t ptr = conf_str;
		config::binding_t<config_t>::parse(ptr, config);
		if(ptr)
			config::error(ptr, "unexpected '}'");
	}
	catch(config::exception_t const &ex) {
		config::report_position(conf_name, conf_str, ex.ptr);
		return 1;
	}

	char obuf[1024];
	out_fd_t out(obuf, sizeof(obuf), 1);

	config::binding_t<config_t>::print(out, 0, config);

	return 0;
}

void phantom_t::syntax() {
	char obuf[1024];
	out_fd_t out(obuf, sizeof(obuf), 1);

	config::syntax_t::push(
		config::binding_t<config_t>::sname,
		&config::binding_t<config_t>::syntax
	);

	config::syntax_t::proc(out);
	config::syntax_t::erase();
}

namespace phantom {

static inline char const *ftype(mode_t mode) {
	switch(mode & S_IFMT) {
		case S_IFDIR: return "directory";
		case S_IFCHR: return "character device";
		case S_IFBLK: return "block device";
		case S_IFREG: return "regular file";
		case S_IFIFO: return "FIFO";
		case S_IFLNK: return "symbolic link";
		case S_IFSOCK: return "socket";
	}
	return "unknown";
}

class fd_list_t {
	struct item_t : public list_item_t<item_t> {
		int fd;

		inline item_t(int _fd, item_t *&list)
			: list_item_t<item_t>(this, list), fd(_fd) { }

		inline bool find(int _fd) {
			if(!this)
				return false;

			if(fd == _fd)
				return true;

			return next->find(_fd);
		}
	};

	item_t *list;

public:
	inline bool find(int _fd) { return list->find(_fd); }

	void store(int _fd) { new item_t(_fd, list); }

	void check(int _fd) {
		if(!find(_fd)) {
			struct stat st;
			char const *_ftype = NULL;

			if(fstat(_fd, &st) < 0)
				log_error("fstat: %m");
			else
				_ftype = ftype(st.st_mode);

			log_error("lost file %u (%s)", _fd, _ftype);
		}
	}

	void foreach(void (fd_list_t::*fun)(int));

	inline fd_list_t() : list(NULL) { foreach(&fd_list_t::store); }

	inline ~fd_list_t() throw() {
		foreach(&fd_list_t::check);

		while(list) delete list;
	}
};

void fd_list_t::foreach(void (fd_list_t::*fun)(int)) {
	int fd = open("/proc/self/fd", O_RDONLY, 0);
	if(fd < 0) {
		log_error("open /proc/self/fd: %m");
		return;
	}

	fd_guard_t fd_guard(fd);

	while(true) {
		off_t unused;
		char buf[1024];
		ssize_t len = getdirentries(fd, buf, sizeof(buf), &unused);

		if(!len)
			break;

		if(len < 0) {
			log_error("getdirentries: %m");
			return;
		}

		ssize_t i = 0;

		while(i < len) {
			dirent *de = (dirent *)(buf + i);
			char const *ptr = de->d_name;

			if(*ptr != '.') {
				int fdn = 0;
				char c;

				while((c = *(ptr++))) {
					assert(c >= '0' && c <= '9');
					fdn = fdn * 10 + (c - '0');
				}

				if(fdn != fd) (this->*fun)(fdn);
			}

			i += de->d_reclen;
		}

		assert(i == len);
	}
}

static char const *usage_str =
	"Usage:\n"
	"\tphantom run <conffile> [args]\n"
	"\tphantom check <conffile> [args]\n"
	"\tphantom syntax [modules]\n"
;

static void usage() {
	size_t __attribute__((unused)) n = ::write(2, usage_str, strlen(usage_str));
}

extern "C" int main(int _argc, char *_argv[], char *_envp[]) {
	--_argc; ++_argv;

	if(!_argc) {
		usage();
		return 0;
	}

	enum { run, check, syntax } mode;

	if(!strcmp(_argv[0], "run"))
		mode = run;
	else if(!strcmp(_argv[0], "check"))
		mode = check;
	else if(!strcmp(_argv[0], "syntax"))
		mode = syntax;
	else {
		usage();
		return 1;
	}

	--_argc; ++_argv;

	try {
		fd_list_t fd_list;

		if(mode == syntax) {
			for(int i = 0; i < _argc; ++i)
				module_load(_argv[i]);

			phantom_t::syntax();

			return 0;
		}
		else {
			if(!_argc) {
				usage();
				return 1;
			}

			string_t name = config::setup(_argc, _argv, _envp);

			return (mode == run ? phantom_t::run : phantom_t::check)(name);
		}
	}
	catch(exception_t const &ex) {
	}
	catch(string_t const &ex) {
		log_error("%s", ex.ptr());
	}
	catch(...) {
		log_error("unknown exception");
	}

	return 1;
}

} // namespace phantom
