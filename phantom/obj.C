// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "obj.H"

#include <pd/bq/bq_thr.H>

#include <pd/base/config_helper.H>
#include <pd/base/log.H>

namespace phantom {

obj_t *obj_t::list = NULL;

void obj_init(obj_t *obj) {
	if(!obj) return;
	obj_init(obj->next);

	try {
		log::handler_t handler(obj->name);
		obj->init();
	}
	catch(...) {
		bq_thr_t::stop();

		obj_fini(obj->next);
		throw;
	}
}

void obj_exec(obj_t const *obj) {
	if(!obj) return;
	obj_exec(obj->next);
	{
		log::handler_t handler(obj->name);
		obj->exec();
	}
}

void obj_stat_print(obj_t const **objs, size_t size) {
	while(size--) {
		obj_t const *obj = *(objs++);
		stat::ctx_t ctx(obj->name.str(), -1);
		obj->stat_print();
	}
}

void obj_fini(obj_t *obj) {
	if(!obj) return;
	{
		log::handler_t handler(obj->name);
		obj->fini();
	}
	obj_fini(obj->next);
}

obj_t const *obj_find(string_t const &name) {
	for(obj_t *obj = obj_t::list; obj; obj = obj->next) {
		if(string_t::cmp_eq<ident_t>(name, obj->name)) return obj;
	}
	return NULL;
}

size_t objs_all(obj_t const **objs, obj_t *obj) {
	if(!obj)
		return 0;

	size_t count = objs_all(objs, obj->next);
	if(objs)
		objs[count] = obj;

	return count + 1;
}

} // namespace phantom

namespace pd { namespace config {

typedef phantom::obj_t obj_t;

template<>
void helper_t<obj_t const *>::parse(in_t::ptr_t &ptr, obj_t const *&val) {
	string_t name = parse_name(ptr);
	obj_t const *obj = phantom::obj_find(name);
	if(!obj)
		error(ptr, "name not found");

	val = obj;
}

template<>
void helper_t<obj_t const *>::print(out_t &out, int, obj_t const *const &val) {
	if(val) {
		out(val->name);
	}
	else
		out(CSTR("NULL"));
}

template<>
void helper_t<obj_t const *>::syntax(out_t &out) {
	out(CSTR("<obj_name>"));
}

}} // namespace pd::config
