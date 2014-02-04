// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "setup.H"
#include "module.H"

#include <pd/base/config.H>
#include <pd/base/config_list.H>
#include <pd/base/exception.H>

#include <dlfcn.h>

// FIXME: abi::__cxa_demangle does not work
// #include <cxxabi.h>

// for freeing memory alloced by dlerror & abi::__cxa_demangle
#include <malloc.h>

namespace phantom {

void module_load(char const *file_name) {
	if(!dlopen(file_name, RTLD_NOW | RTLD_GLOBAL)) {
		char *err = dlerror();
/*
		if(err) {
			char *demangled = abi::__cxa_demangle(err, NULL, 0, NULL);
			if(demangled) {
				free(err); err = demangled;
			}
		}
*/
		if(!err)
			throw STRING("Unknown libdl error");

		string_t::ctor_t error_z(strlen(err) + 1);
		error_z(str_t(err, strlen(err)))('\0');

		free(err);

		throw string_t(error_z);
	}
}

class setup_module_t : public setup_t {
public:
	struct name_t : public string_t {
		in_t::ptr_t ptr;
		inline name_t() : string_t(), ptr(string_t::empty) { }

		inline name_t(string_t const &str, in_t::ptr_t _ptr) :
			string_t(str), ptr(_ptr) { }

		inline ~name_t() throw() { }
	};

	struct config_t {
		string_t dir;
		config::list_t<name_t> list;

		inline config_t() throw() : dir(STRING(".")) { }
		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &) const {
			for(typeof(list._ptr()) lptr = list; lptr; ++lptr) {
				name_t const &name = lptr.val();

				// #dir "/mod_" #name ".so\0"

				if(module_info_t::lookup(name)) return;

				string_t fname_z = string_t::ctor_t(dir.size() + 5 + name.size() + 4)
					(dir)(CSTR("/mod_"))(name)(CSTR(".so\0"))
				;

				try {
					module_load(fname_z.ptr());
				}
				catch(string_t const &ex) {
					config::error(name.ptr, ex.ptr());
				}

				module_info_t *module_info = module_info_t::lookup(name);
				if(!module_info)
					config::error(name.ptr, "not a phantom module loaded");

				if(!*module_info)
					config::error(name.ptr, "illegal module version");
			}
		}
	};

	inline setup_module_t(string_t const &, config_t const &) throw() { }
	inline ~setup_module_t() throw() { }
};

namespace setup_module {
config_binding_sname(setup_module_t);
config_binding_value(setup_module_t, dir);
config_binding_value(setup_module_t, list);
config_binding_ctor(setup_t, setup_module_t);
}

} // namespace phantom

namespace pd { namespace config {

typedef phantom::setup_module_t::name_t name_t;

template<>
void helper_t<name_t>::parse(in_t::ptr_t &ptr, name_t &name) {
	in_t::ptr_t p = ptr;
	name = name_t(parse_name(ptr), p);
}

template<>
void helper_t<name_t>::print(out_t &out, int, name_t const &name) {
	out(name);
}

template<>
void helper_t<name_t>::syntax(out_t &out) {
	out(CSTR("<module_name>"));
}

}} // namespace pd::config
