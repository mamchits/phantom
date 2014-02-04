// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "setup.H"

#include <pd/base/size.H>
#include <pd/base/config.H>
#include <pd/base/config_list.H>
#include <pd/base/config_record.H>
#include <pd/base/config_enum.H>
#include <pd/base/exception.H>

#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/resource.h>

#ifdef __linux__
#define LINUX_CAPS (1)
#else
#define LINUX_CAPS (0)
#endif

#if LINUX_CAPS

#include <sys/prctl.h>
#include <linux/capability.h>

extern "C" int capget(__user_cap_header_struct *, __user_cap_data_struct *);
extern "C" int capset(__user_cap_header_struct *, __user_cap_data_struct *);

#endif

namespace phantom {

class setup_caps_t : public setup_t {
public:
	struct user_t : public string_t {
		uid_t id;

		inline user_t() throw() : string_t(), id(0) { }
		inline ~user_t() throw() { }

		inline void setup(in_t::ptr_t &ptr) {
			if(!*this)
				return;

			MKCSTR(_user, *this);

			struct passwd *pw = getpwnam(_user);
			if(!pw)
				config::error(ptr, "user does not exist");

			id = pw->pw_uid;
		}
	};

	struct group_t : public string_t {
		gid_t id;

		inline group_t() throw() : string_t(), id(0) { }
		inline ~group_t() throw() { }

		inline void setup(in_t::ptr_t &ptr) {
			if(!*this)
				return;

			MKCSTR(_group, *this);

			struct group *gr = getgrnam(_group);
			if(!gr)
				config::error(ptr, "group does not exist");

			id = gr->gr_gid;
		}
	};

	struct capnum_t {
		unsigned int v;
	};

	struct caps_t : public config::list_t<capnum_t> {
		uint32_t value;

		inline caps_t() throw() : config::list_t<capnum_t>(), value(0) { }
		inline ~caps_t() throw() { }

		inline void setup(in_t::ptr_t &ptr) {
			value = 0;

#if LINUX_CAPS
			for(ptr_t lptr = *this; lptr; ++lptr) {
				unsigned int v = lptr.val().v;
				if(v > 31)
					config::error(ptr, "capability number is unknown");

				value |= (1U << v);
			}
#else
			if(*this)
				config::error(ptr, "keeping capabilities is not implemented for this OS");
#endif
		}
	};

	struct resource_t {
		enum id_t {
			cpu = RLIMIT_CPU,
			fsize = RLIMIT_FSIZE,
			data = RLIMIT_DATA,
			stack = RLIMIT_STACK,
			core = RLIMIT_CORE,
			// rss = RLIMIT_RSS,
			nproc = RLIMIT_NPROC,
			nofile = RLIMIT_NOFILE,
			memlock = RLIMIT_MEMLOCK,
			as = RLIMIT_AS,
			// locks = RLIMIT_LOCKS,
			sigpending = RLIMIT_SIGPENDING,
			msgqueue = RLIMIT_MSGQUEUE,
			nice = RLIMIT_NICE,
			rtprio = RLIMIT_RTPRIO,
			unset = RLIM_NLIMITS
		};

		config::enum_t<id_t> id;
		sizeval_t val;

		inline resource_t() throw() : id(unset), val() { }
		inline ~resource_t() throw() { }
	};

	typedef config::record_t<resource_t> rlimit_t;

	struct config_t {
		user_t user;
		group_t group;
		caps_t keep;
		config::list_t<rlimit_t> rlimits;

		inline config_t() throw() : user(), group(), keep() { }
		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &) const { }
	};

	inline setup_caps_t(string_t const &, config_t const &config) {
		if(config::check)
			return;

#if LINUX_CAPS
		uint32_t caps_keep = config.keep.value;

		if(caps_keep) {
			if(prctl(PR_SET_KEEPCAPS, 1) < 0)
					throw exception_sys_t(log::error, errno, "prctl(PR_SET_KEEPCAPS, 1): %m");
		}
#endif

		for(config::list_t<rlimit_t>::ptr_t rptr = config.rlimits; rptr; ++rptr) {
			rlimit_t const &rlim = rptr.val();
			struct rlimit r;

			r.rlim_cur = r.rlim_max = rlim.val < sizeval::unlimited ? (rlim_t)rlim.val : RLIM_INFINITY;

			if(setrlimit(rlim.id, &r) < 0)
				throw exception_sys_t(log::error, errno, "setrlimit: %m");
		}

		gid_t gid = config.group.id;

		if(gid) {
			if(setgroups(1, &gid) < 0)
				throw exception_sys_t(log::error, errno, "setgroups: %m");

			if(setregid(gid, gid) < 0)
				throw exception_sys_t(log::error, errno, "setregid: %m");
		}

		uid_t uid = config.user.id;

		if(uid) {
			if(setreuid(uid, uid) < 0)
				throw exception_sys_t(log::error, errno, "setreuid: %m");
		}

#if LINUX_CAPS
		if(caps_keep) {
			__user_cap_header_struct hdr;
			__user_cap_data_struct data;

			hdr.version = _LINUX_CAPABILITY_VERSION;
			hdr.pid = 0;

			data.effective = data.permitted = caps_keep;
			data.inheritable = 0;

			if(::capset(&hdr, &data) < 0)
				throw exception_sys_t(log::error, errno, "capset: %m");

			if(prctl(PR_SET_KEEPCAPS, 0) < 0)
				throw exception_sys_t(log::error, errno, "prctl(PR_SET_KEEPCAPS, 0): %m");

			if(prctl(PR_SET_DUMPABLE, 1) < 0)
				throw exception_sys_t(log::error, errno, "prctl(PR_SET_DUMPABLE, 1): %m");
		}
#endif
	}

	inline ~setup_caps_t() throw() { }
};

namespace setup_cups {
config_binding_sname(setup_caps_t);
config_binding_value(setup_caps_t, user);
config_binding_value(setup_caps_t, group);
config_binding_value(setup_caps_t, keep);
config_binding_value(setup_caps_t, rlimits);
config_binding_ctor(setup_t, setup_caps_t);

namespace resource {
config_enum_internal_sname(setup_caps_t::resource_t, id_t);
config_enum_internal_value(setup_caps_t::resource_t, id_t, cpu);
config_enum_internal_value(setup_caps_t::resource_t, id_t, fsize);
config_enum_internal_value(setup_caps_t::resource_t, id_t, data);
config_enum_internal_value(setup_caps_t::resource_t, id_t, stack);
config_enum_internal_value(setup_caps_t::resource_t, id_t, core);
config_enum_internal_value(setup_caps_t::resource_t, id_t, nproc);
config_enum_internal_value(setup_caps_t::resource_t, id_t, nofile);
config_enum_internal_value(setup_caps_t::resource_t, id_t, memlock);
config_enum_internal_value(setup_caps_t::resource_t, id_t, as);
config_enum_internal_value(setup_caps_t::resource_t, id_t, sigpending);
config_enum_internal_value(setup_caps_t::resource_t, id_t, msgqueue);
config_enum_internal_value(setup_caps_t::resource_t, id_t, nice);
config_enum_internal_value(setup_caps_t::resource_t, id_t, rtprio);

config_record_sname(setup_caps_t::resource_t);
config_record_value(setup_caps_t::resource_t, id);
config_record_value(setup_caps_t::resource_t, val);
}}

} // namespace phantom

namespace pd { namespace config {

typedef phantom::setup_caps_t::user_t user_t;

template<>
void helper_t<user_t>::parse(in_t::ptr_t &ptr, user_t &val) {
	helper_t<string_t>::parse(ptr, val);
	val.setup(ptr);
}

template<>
void helper_t<user_t>::print(out_t &out, int off, user_t const &val) {
	 helper_t<string_t>::print(out, off, val);
}

template<>
void helper_t<user_t>::syntax(out_t &out) {
	helper_t<string_t>::syntax(out);
}

typedef phantom::setup_caps_t::group_t group_t;

template<>
void helper_t<group_t>::parse(in_t::ptr_t &ptr, group_t &val) {
	helper_t<string_t>::parse(ptr, val);
	val.setup(ptr);
}

template<>
void helper_t<group_t>::print(out_t &out, int off, group_t const &val) {
	 helper_t<string_t>::print(out, off, val);
}

template<>
void helper_t<group_t>::syntax(out_t &out) {
	helper_t<string_t>::syntax(out);
}

typedef phantom::setup_caps_t::capnum_t capnum_t;

template<>
void helper_t<capnum_t>::parse(in_t::ptr_t &ptr, capnum_t &num) {
	helper_t<unsigned int>::parse(ptr, num.v);
}

template<>
void helper_t<capnum_t>::print(out_t &out, int off, capnum_t const &num) {
	helper_t<unsigned int>::print(out, off, num.v);
}

template<>
void helper_t<capnum_t>::syntax(out_t &out) {
	helper_t<unsigned int>::syntax(out);
}

typedef phantom::setup_caps_t::caps_t caps_t;

template<>
void helper_t<caps_t>::parse(in_t::ptr_t &ptr, caps_t &val) {
	helper_t<list_t<capnum_t>>::parse(ptr, val);
	val.setup(ptr);
}

template<>
void helper_t<caps_t>::print(out_t &out, int off, caps_t const &val) {
	 helper_t<list_t<capnum_t>>::print(out, off, val);
}

template<>
void helper_t<caps_t>::syntax(out_t &out) {
	helper_t<list_t<capnum_t>>::syntax(out);
}

}} // namespace pd::config
