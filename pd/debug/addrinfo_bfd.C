// This file is part of the pd::debug library.
// Copyright (C) 2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "addrinfo_bfd.H"

#include <pd/base/trace.H>
#include <pd/base/list.H>
#include <pd/base/exception.H>

#include <bfd.h>
#include <cxxabi.h>
#include <stdlib.h>

namespace pd { namespace debug {

class bfd_t {
	class demangle_t {
		char *data;
		size_t size;

	public:
		inline demangle_t() throw() : data(NULL), size(0) { }
		inline ~demangle_t() throw() { if(data) ::free(data); }

		char const *operator()(char const *func) {
			char *tmp = abi::__cxa_demangle(func, data, &size, NULL);

			return tmp ? (data = tmp) : func;
		}
	};

	demangle_t demangle;

	struct file_t : list_item_t<file_t> {
		char *name;

		bfd *abfd;
		asymbol **syms;
		asection **sections;
		unsigned int section_count;

		void setup();

		inline file_t(char const *_name, file_t *&list) :
			list_item_t<file_t>(this, list), name(strdup(_name)),
			abfd(NULL), syms(NULL), sections(NULL), section_count(0) {

			setup();
		}

		inline ~file_t() throw() {
			if(sections)
				::free(sections);

			if(syms)
				::free(syms);

			if(abfd)
				bfd_close(abfd);

			::free(name);
		}

		inline void *operator new(size_t sz) { return ::malloc(sz); }
		inline void operator delete(void *ptr) { ::free(ptr); }

		file_t *find(char const *_name) {
			return (!this || !strcmp(name, _name))
				? this
				: next->find(_name)
			;
		}

		void print(uintptr_t addr, uintptr_t addr_rel, demangle_t &demangle, out_t &out);
	};

	file_t *list;

public:
	inline bfd_t() throw() : list(NULL) {
		bfd_init();
	}

	inline ~bfd_t() throw() { while(list) delete(list); }

	inline void *operator new(size_t sz) { return ::malloc(sz); }
	inline void operator delete(void *ptr) { ::free(ptr); }

	inline void print(
		uintptr_t addr, uintptr_t addr_rel, char const *fname, out_t &out
	) {
		file_t *file = list->find(fname) ?: new file_t(fname, list);

		file->print(addr, addr_rel, demangle, out);
	}
};

static asymbol **get_syms(bfd *_abfd, bool dyn) {
	long storage = dyn
		? bfd_get_dynamic_symtab_upper_bound(_abfd)
		: bfd_get_symtab_upper_bound(_abfd)
	;

	if(storage <= 0)
		return NULL;

	asymbol **_syms = (asymbol **)::malloc(storage);

	long symcount = dyn
		? bfd_canonicalize_dynamic_symtab(_abfd, _syms)
		: bfd_canonicalize_symtab(_abfd, _syms)
	;

	if(symcount <= 0) {
		::free(_syms);
		_syms = NULL;
	}

	return _syms;
}

void bfd_t::file_t::setup() {
	// FIXME: path may be relative and cwd changed

	bfd *_abfd = bfd_openr(name, NULL /* target */);
	if(!_abfd)
		return;

	if(!bfd_check_format(_abfd, bfd_object)) {
		bfd_close(_abfd);
		return;
	}

	asymbol **_syms = get_syms(_abfd, false);
	if(!_syms) _syms = get_syms(_abfd, true);

	if(!_syms) {
		bfd_close(_abfd);
		return;
	}

	unsigned int _section_count = 0;

	for(asection *sect = _abfd->sections; sect != NULL; sect = sect->next) {
		if(!(bfd_get_section_flags(_abfd, sect) & SEC_CODE))
			continue;

		++_section_count;
	}

	asection **_sections = (asection **)::malloc(_section_count * sizeof(asection *));
	asection **sectp = _sections;

	for(asection *sect = _abfd->sections; sect != NULL; sect = sect->next) {
		if(!(bfd_get_section_flags(_abfd, sect) & SEC_CODE))
			continue;

		*(sectp++) = sect;
	}

	abfd = _abfd;
	syms = _syms;
	sections = _sections;
	section_count = _section_count;
}

void bfd_t::file_t::print(
	uintptr_t addr, uintptr_t addr_rel, demangle_t &demangle, out_t &out
) {
	if(!abfd)
		return;

	bfd_vma pc = bfd_get_file_flags(abfd) & DYNAMIC
		? addr_rel
		: addr
	;

	for(unsigned int i = 0; i < section_count; ++i) {
		asection *sect = sections[i];

		bfd_vma vma = bfd_get_section_vma(abfd, sect);

		if(pc < vma)
			continue;

		bfd_size_type size = bfd_get_section_size(sect);
		if(pc >= vma + size)
			continue;

		char const *filename, *func;
		unsigned int lineno = 0;

		if(
			bfd_find_nearest_line(
				abfd, sect, syms, pc - vma, &filename, &func, &lineno
			) && func
		) {
			do {
				char const *pfunc = demangle(func);

				out(' ')(' ')(str_t(pfunc, strlen(pfunc))).lf();

				if(filename)
					out
						(' ')(' ')(' ')(str_t(filename, strlen(filename)))(':')
						.print(lineno).lf()
				;

			} while(bfd_find_inliner_info(abfd, &filename, &func, &lineno) && func);
		}
		break;
	}
}

static bfd_t *bfd;

void trace_addrinfo_print_bfd(
	uintptr_t addr, uintptr_t addr_rel, char const *fname, out_t &out
) {
	try {
		bfd->print(addr, addr_rel, fname, out);
	}
	catch(exception_t const &ex) {
		out(CSTR("Error: "))(ex.msg()).lf();
	}
	catch(...) {
		out(CSTR("Error: unknown exception")).lf();
	}
}

struct mgr_t {
	trace_addrinfo_print_t orig;

	inline mgr_t() throw() : orig(trace_addrinfo_print) {
		bfd = new bfd_t;
		trace_addrinfo_print = &trace_addrinfo_print_bfd;
	}

	inline ~mgr_t() throw() {
		// trace_addrinfo_hook = orig;
		// delete bfd;
	}
};

static mgr_t const mgr;

}} // namespace pd::debug
