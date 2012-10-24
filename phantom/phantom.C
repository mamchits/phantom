// This file is part of the phantom program.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "setup.H"
#include "io.H"
#include "logger.H"
#include "module.H"

#include <pd/bq/bq_spec.H>

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

enum time_format_t { brief = 0, full };

config_enum_sname(time_format_t);
config_enum_value(time_format_t, brief);
config_enum_value(time_format_t, full);

class phantom_t {
public:
	typedef phantom::obj_t obj_t;
	typedef phantom::scheduler_t scheduler_t;
	typedef phantom::io_t io_t;
	typedef phantom::logger_t logger_t;
	typedef phantom::setup_t setup_t;

	struct stat_config_t {
		config::enum_t<bool> clear;
		config::enum_t<time_format_t> time_format;
		interval_t period;
		string_t filename;
		config::list_t<obj_t *> list;

		inline stat_config_t() throw() :
			clear(false), time_format(brief), period(interval_zero),
			filename(), list() { }

		inline void check(in_t::ptr_t const &) const { }

		inline ~stat_config_t() throw() { }
	};

	struct config_t {
		config_binding_type_ref(setup_t);
		config_binding_type_ref(scheduler_t);
		config_binding_type_ref(logger_t);
		config_binding_type_ref(io_t);

		config::objptr_t<logger_t> logger;

		config::struct_t<stat_config_t> stat;

		inline config_t() : logger(), stat() { }

		inline ~config_t() { }

		inline void check(in_t::ptr_t const &) const { }

		void stat_print(out_t &out, bool force_total = false);
	};

	static void run(string_t const &name);
	static void check(string_t const &name);
	static void syntax();
};

namespace phantom_stat {
config_binding_sname_sub(phantom_t, stat);
config_binding_value_sub(phantom_t, stat, clear);
config_binding_value_sub(phantom_t, stat, time_format);
config_binding_value_sub(phantom_t, stat, period);
config_binding_value_sub(phantom_t, stat, filename);
config_binding_value_sub(phantom_t, stat, list);
}

namespace phantom {
config_binding_sname(phantom_t);
config_binding_type(phantom_t, setup_t);
config_binding_type(phantom_t, scheduler_t);
config_binding_type(phantom_t, logger_t);
config_binding_value(phantom_t, logger);
config_binding_type(phantom_t, io_t);
config_binding_value(phantom_t, stat);
}

void phantom_t::config_t::stat_print(out_t &out, bool force_total) {
	bool stat_clear = stat.clear && !force_total;

	out(CSTR("time\t")).print(
		timeval_current(), stat.time_format == full ? "+dz" : "+"
	);

	if(!stat_clear)
		out(CSTR("\t(total)"));

	out.lf();

	for(typeof(stat.list.ptr()) ptr = stat.list; ptr; ++ptr) {
		obj_t *obj = ptr.val();
		out.lf()(CSTR("** "))(obj->name).lf();
		obj->stat(out, stat_clear);
		out.lf();
	}

/*
	if(stat.system) {
		bq_stack_pool_info_t info = bq_stack_pool_get_info();

		out
			.lf()
			(CSTR("stack pool: "))
			.print(info.wsize)('/')
			.print(info.size).lf().lf()
		;
	}
 */
}

namespace phantom {

bq_spec_decl(log::handler_t const, log_handler_current);

class log_mgr_t {
	static log_mgr_t const instance;

	static inline log::handler_t const *&current() {
		return log_handler_current;
	}

	static inline void set(log::handler_t const *log_handler) {
		current() = log_handler;
	}

	static inline log::handler_t const *get() {
		return current();
	}

	inline log_mgr_t() throw() {
		log::handler_t::init(&set, &get);
	}

	inline ~log_mgr_t() throw() {
		log::handler_t::init(NULL, NULL);
	}
};

log_mgr_t const __init_priority(102) log_mgr_t::instance;

} // namespace phantom

void phantom_t::run(string_t const &conf_name) {
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
		return;
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

	log::handler_default_t log_handler(STRING("phantom"), config.logger);

	log_info("Start");

	try {
		int stat_fd;

		if(config.stat.filename) {
			string_t filename_z = ({
				string_t::ctor_t ctor(config.stat.filename.size() + 1);
				ctor(config.stat.filename)('\0');
				(string_t)ctor;
			});

			stat_fd = ::open(filename_z.ptr(), O_WRONLY | O_CREAT | O_APPEND, 0644);
			if(stat_fd < 0)
				throw exception_sys_t(
					log::error, errno, "open (%s): %m", filename_z.ptr()
				);
		}
		else {
			stat_fd = ::dup(1);
			if(stat_fd < 0)
				throw exception_sys_t(log::error, errno, "dup: %m");
		}

		fd_guard_t stat_fd_guard(stat_fd);

		struct obj_guard_t {
			inline obj_guard_t() { phantom::obj_init(); }

			inline ~obj_guard_t() {
				bq_thr_t::stop();

				phantom::obj_fini();
			}
		};

		obj_guard_t obj_guard;

		bool need_stat = config.stat.list;
		if(need_stat) {
			interval_t period = config.stat.period;
			if(period > interval_zero) {
				time_t s = period / interval_second;
				suseconds_t ms = (period % interval_second) / interval_microsecond;
				itimerval it = { { s, ms }, { s, ms } };
				setitimer(ITIMER_REAL, &it, NULL);
			}
		}

		phantom::obj_exec();

		for(bool work = true; work;) {
			switch(({ int sig; sigwait(&sigset, &sig); sig; })) {
				case SIGQUIT:
				case SIGTERM:
					work = false;
				case SIGINT:
				case SIGALRM:
				if(need_stat) {
					char obuf[1024];
					out_fd_t out(obuf, sizeof(obuf), stat_fd);

					config.stat_print(out);
				}
			}
		}

		if(need_stat && config.stat.clear) {
			char obuf[1024];
			out_fd_t out(obuf, sizeof(obuf), 1);

			config.stat_print(out, true);
		}
	}
	catch(exception_t const &ex) {
		ex.log();
	}
	catch(...) {
		log_error("unknown exception");
	}

	log_info("Exit");
}

void phantom_t::check(string_t const &conf_name) {
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
		return;
	}

	char obuf[1024];
	out_fd_t out(obuf, sizeof(obuf), 1);

	config::binding_t<config_t>::print(out, 0, config);
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
	size_t __attribute__((unused)) n = write(2, usage_str, strlen(usage_str));
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
		}
		else {
			if(!_argc) {
				usage();
				return 1;
			}

			string_t name = config::setup(_argc, _argv, _envp);

			(mode == run ? phantom_t::run : phantom_t::check)(name);
		}

		return 0;
	}
	catch(exception_t const &ex) {
		ex.log();
	}
	catch(...) {
		log_error("unknown exception");
	}

	return 1;
}

} // namespace phantom
