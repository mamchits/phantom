// This file is part of the phantom program.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "obj.H"

#include <pd/bq/bq_thr.H>

#include <pd/base/config_helper.H>

namespace phantom {

obj_t *obj_t::list = NULL;

void obj_init(obj_t *obj) {
	if(!obj) return;
	obj_init(obj->next);

	try {
		obj->init();
	}
	catch(...) {
		bq_thr_t::stop();

		obj_fini(obj->next);
		throw;
	}
}

void obj_exec(obj_t *obj) {
	if(!obj) return;
	obj_exec(obj->next);
	obj->exec();
}

void obj_fini(obj_t *obj) {
	if(!obj) return;
	obj->fini();
	obj_fini(obj->next);
}

obj_t *obj_find(string_t const &name) {
	for(obj_t *obj = obj_t::list; obj; obj = obj->next) {
		if(string_t::cmp_eq<ident_t>(name, obj->name)) return obj;
	}
	return NULL;
}

} // namespace phantom

namespace pd { namespace config {

typedef phantom::obj_t obj_t;

template<>
void helper_t<obj_t *>::parse(in_t::ptr_t &ptr, obj_t *&val) {
	string_t name = parse_name(ptr);
	obj_t *obj = phantom::obj_find(name);
	if(!obj)
		error(ptr, "name not found");

	val = obj;
}

template<>
void helper_t<obj_t *>::print(out_t &out, int, obj_t *const &val) {
	if(val) {
		out(val->name);
	}
	else
		out(CSTR("NULL"));
}

template<>
void helper_t<obj_t *>::syntax(out_t &out) {
	out(CSTR("<obj_name>"));
}

}} // namespace pd::config

