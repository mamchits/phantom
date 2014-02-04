// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "setup.H"
#include "obj.H"

#include <pd/base/config.H>
#include <pd/base/config_enum.H>
#include <pd/base/exception.H>
#include <pd/base/out_fd.H>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/prctl.h>

namespace phantom {

class setup_daemon_t : public setup_t {
	class file_t {
		string_t name_z;
		int fd;

	public:
		inline file_t(string_t const &name) :
			name_z(string_t::ctor_t(name.size() + 1)(name)('\0')) {

			char const *_name = name_z.ptr();

			fd = open(_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
			if(fd < 0)
				throw exception_sys_t(log::error, errno, "open (%s): %m", _name);

			if(flock(fd, LOCK_EX | LOCK_NB) < 0) {
				::close(fd);
				throw exception_sys_t(log::error, errno, "flock (%s): %m", _name);
			}
		}

		void write_pid() {
			if(ftruncate(fd, 0) < 0) {
				throw exception_sys_t(log::error, errno, "ftruncate (%s): %m", name_z.ptr());
			}

			char buf[16]; out_fd_t out(buf, sizeof(buf), fd);
			out.print(getpid()).lf();
			out.flush_all();
		}

		inline ~file_t() throw() {
			unlink(name_z.ptr());
			::close(fd);
		}
	} *file;

public:
	enum coredump_t {
		off = 0, once = 1, on = 2
	};

	struct config_t {
		string_t pidfile;
		config::enum_t<bool> respawn;
		string_t stdout;
		string_t stderr;
		config::enum_t<coredump_t> coredump;

		inline config_t() :
			pidfile(), respawn(false),
			stdout(STRING("/dev/null")), stderr(STRING("/dev/null")), coredump(on) { }

		inline ~config_t() { }

		inline void check(in_t::ptr_t const &ptr) const {
			if(!pidfile)
				config::error(ptr, "pidfile is required");

			if(objs_exist())
				config::error(ptr, "setup_daemon is defined after the first active object");
		}
	};

	inline setup_daemon_t(string_t const &, config_t const &config) : file(NULL) {
		if(config::check)
			return;

		file = new file_t(config.pidfile);

		char stdout_name[config.stdout.size() + 1];
		{
			out_t out(stdout_name, sizeof(stdout_name));
			out(config.stdout)('\0');
		}

		char stderr_name[config.stderr.size() + 1];
		{
			out_t out(stderr_name, sizeof(stderr_name));
			out(config.stderr)('\0');
		}

		try {
			{
				int fd0;
				fd0 = open("/dev/null", O_RDONLY, 0);

				if(fd0 < 0)
					throw exception_sys_t(log::error, errno, "open (/dev/null): %m");

				dup2(fd0, 0);
				close(fd0);

				fd0 = open(stdout_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
				if(fd0 < 0)
					throw exception_sys_t(log::error, errno, "open (%s): %m", stdout_name);
				dup2(fd0, 1);
				close(fd0);

				fd0 = open(stderr_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
				if(fd0 < 0)
					throw exception_sys_t(log::error, errno, "open (%s): %m", stderr_name);
				dup2(fd0, 2);
				close(fd0);
			}

			{
				pid_t pid = fork();
				if(pid < 0) throw exception_sys_t(log::error, errno, "fork: %m");
				if(pid > 0) _exit(0);
			}

			setsid();

			{
				pid_t pid = fork();
				if(pid < 0) throw exception_sys_t(log::error, errno, "fork: %m");
				if(pid > 0) _exit(0);
			}

			if(prctl(PR_SET_DUMPABLE, config.coredump != off) < 0)
				throw exception_sys_t(log::error, errno, "prctl(PR_SET_DUMPABLE, ...): %m");

			if(config.respawn) {
				while(true) {
					pid_t pid = fork();
					if(pid < 0) throw exception_sys_t(log::error, errno, "fork: %m");
					if(pid == 0) break;
					if(pid > 0) {
						int status = 0;
						if(waitpid(pid, &status, 0) < 0)
							throw exception_sys_t(log::error, errno, "waitpid: %m");

						int sig = WTERMSIG(status);

						if(!sig)
							_exit(0);

						int core = WCOREDUMP(status);

						log_error(
							"phantom killed: %s (core %sdumped)",
							strsignal(sig), (core ? "" : "not ")
						);

						if(core && config.coredump == once)
							if(prctl(PR_SET_DUMPABLE, 0) < 0)
								log_error("prctl(PR_SET_DUMPABLE, 0): %m");
					}
				}
			}

			file->write_pid();
		}
		catch(...) {
			delete file;
			throw;
		}
	}

	inline ~setup_daemon_t() throw() {
		if(config::check)
			return;

		delete file;
	}
};

namespace setup_daemon {
config_binding_sname(setup_daemon_t);
config_binding_value(setup_daemon_t, pidfile);
config_binding_value(setup_daemon_t, respawn);
config_binding_value(setup_daemon_t, stdout);
config_binding_value(setup_daemon_t, stderr);
config_binding_value(setup_daemon_t, coredump);
config_binding_ctor(setup_t, setup_daemon_t);

config_enum_internal_sname(setup_daemon_t, coredump_t);
config_enum_internal_value(setup_daemon_t, coredump_t, off);
config_enum_internal_value(setup_daemon_t, coredump_t, once);
config_enum_internal_value(setup_daemon_t, coredump_t, on);
}

} // namespace phantom
