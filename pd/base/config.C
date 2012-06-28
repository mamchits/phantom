// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "config.H"
#include "config_enum.H"
#include "out_fd.H"

#include <stdio.h>

namespace pd { namespace config {

static inline bool is_name_start_char(char c) {
	switch(c) {
		case 'a' ... 'z': case 'A' ... 'Z': case '_':
			return true;
	}
	return false;
}

static inline bool is_name_char(char c) {
	switch(c) {
		case 'a' ... 'z': case 'A' ... 'Z': case '0' ... '9': case '_':
			return true;
	}
	return false;
}

static inline bool is_space(char c) {
	switch(c) {
		case ' ': case '\t': case '\r': case '\n': case '}': case '\0':
			return true;
	}
	return false;
}

char skip_space(in_t::ptr_t &ptr) {
	while(ptr) {
		char c = *ptr;
		switch(c) {
			case '#':
				while(true) {
					if(!++ptr) return '\0';
					if((c = *ptr) == '\n') break;
				}
			case '\n': case ' ': case '\t': case '\r':
				++ptr; break;
			case '}':
				return '\0';
			default:
				return c;
		}
	}
	return '\0';
}

string_t parse_name(in_t::ptr_t &ptr) {
	in_t::ptr_t p = ptr;
	in_t::ptr_t p0 = p;

	if(!p || !is_name_start_char(*p))
		error(ptr, "name is expected");

	while(p && is_name_char(*p)) ++p;

	ptr = p;
	return string_t(p0, p - p0);
}

void block_t::parse(in_t::ptr_t &ptr) {
	in_t::ptr_t p = ptr;

	switch(skip_space(p)) {
		case '{':
			parse_content(++p);

			if(!p.match<ident_t>('}'))
				error(p, "'}' is expected");

			break;

		default:
			error(p, "'{' is expected");
	}

	ptr = p;
}

class environ_t {
	class item_t : public list_item_t<item_t> {
		string_t key;
		string_t val;

	public:
		inline item_t(string_t const &_key, string_t const &_val, item_t *&list) :
			list_item_t<item_t>(this, list),
			key(_key), val(_val) { }

		inline ~item_t() throw() { }

		inline string_t const *lookup(string_t const &_key) const {
			if(!this)
				return NULL;

			if(string_t::cmp_eq<ident_t>(key, _key))
				return &val;

			return next->lookup(_key);
		}
	};

	item_t *list;

public:
	inline environ_t() throw() : list(NULL) { }
	inline ~environ_t() throw() { while(list) delete list; }

	inline void setup(char *_envp[]) {
		while(list) delete list;

		for(char **e = _envp; *e; ++e) {
			size_t len = strlen(*e);

			string_t env = string_t::ctor_t(len)(str_t(*e, len));

			in_t::ptr_t p0 = env;
			in_t::ptr_t p = p0;
			if(!p.scan("=", 1, len))
				continue;

			string_t key = string_t(p0, p - p0);
			++p; --len;
			string_t val = string_t(p, len);

			new item_t(key, val, list);
		}
	}

	inline string_t const *lookup(string_t const &key) const {
		return list->lookup(key);
	}
};

static environ_t environ;

class args_t {
	size_t c;
	string_t *v;

public:
	inline args_t() : c(0), v(NULL) { }
	inline ~args_t() throw() { if(v) delete [] v; }

	inline void setup(size_t _argc, char *_argv[]) {
		if(v) delete [] v;
		v = NULL;

		if((c = _argc)) {
			v = new string_t[c];

			for(size_t i = 0; i < c; ++i) {
				char const *a = _argv[i];
				size_t len = strlen(a);

				v[i] = string_t::ctor_t(len)(str_t(a, len));
			}
		}
	}

	inline string_t const *operator[](size_t ind) const {
		return ind < c ? &v[ind] : NULL;
	}
};

static args_t args;

string_t setup(int _argc, char *_argv[], char *envp[]) {
	args.setup(_argc, _argv);
	environ.setup(envp);
	string_t const *res = args[0];
	return res ? *res : string_t::empty;
}

void value_parser_base_t::parse(in_t::ptr_t &ptr) {
	in_t::ptr_t p = ptr;

	if(skip_space(p) != '=')
		error(p, "'=' is expected");

	switch(skip_space(++p)) {
		case '\0':
			error(p, "value is expected");

		case '$': {
			in_t::ptr_t p0 = p;
			++p;
			string_t str;

			if(*p >= '0' && *p <= '9') {
				unsigned int num;
				helper_t<unsigned int>::parse(p, num);

				string_t const *val = args[num];
				if(!val)
					error(p0, "illegal argument number");

				str = *val;
			}
			else {
				string_t const *val = environ.lookup(parse_name(p));
				if(!val)
					error(p0, "unknown evironment variable");

				str = *val;
			}

			try {
				in_t::ptr_t _ptr(str);
				parse_value(_ptr);
				if(_ptr)
					error(ptr, "junk at the end");
			}
			catch(exception_t const &ex) {
				report_position(string_t(p0, p - p0), str, ex.ptr);
				throw exception_t(p);
			}
		}
		break;

		default:
			parse_value(p);
			if(p && !is_space(*p))
				error(p, "junk at the end of value");

		break;
	}

	ptr = p;
}

void print_off(out_t &out, int off) {
	for(int i = 0; i < off; ++i) out(' ');
}

bool check = false;

void error(in_t::ptr_t const &ptr, char const *msg) {
	char buf[1024];
	out_fd_t out(buf, sizeof(buf), 2);

	out(CSTR("config error: "))(str_t(msg, strlen(msg))).lf();

	throw exception_t(ptr);
}

void report_position(string_t const &name, in_t const &in, in_t::ptr_t const &ptr) {
	in_t::ptr_t p0 = in;
	size_t off = ptr - p0; size_t lineno = 1;

	while(p0.scan("\n", 1, off) && off) {
		++lineno; ++p0; --off;
	}

	char buf[1024];
	out_fd_t out(buf, sizeof(buf), 2);

	out(CSTR("\tat the '"))(name)('\'')(' ').print(lineno)(',').print(ptr - p0 + 1).lf();
}

void report_obj(string_t const &obj_name) {
	char buf[1024];
	out_fd_t out(buf, sizeof(buf), 2);
	out(CSTR("\tin the object "))(obj_name).lf();
}

config_enum_sname(bool);
config_enum_value(bool, false);
config_enum_value(bool, true);

}} // namespace pd::config
