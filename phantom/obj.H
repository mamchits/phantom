// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/list.H>
#include <pd/base/string.H>

#pragma GCC visibility push(default)

namespace phantom {

class obj_t : public list_item_t<obj_t> {
	static obj_t *list;

	virtual void init() = 0;
	virtual void exec() const = 0;
	virtual void stat_print() const = 0;
	virtual void fini() = 0;

public:
	string_t const name;

	friend void obj_init(obj_t *obj);
	friend void obj_exec(obj_t const *obj);
	friend void obj_stat_print(obj_t const **objs, size_t size);
	friend void obj_fini(obj_t *obj);

	friend obj_t const *obj_find(string_t const &name);
	friend bool objs_exist();
	friend size_t objs_all(obj_t const **objs, obj_t *obj);

protected:
	inline obj_t(string_t const &_name) :
		list_item_t<obj_t>(this, list), name(_name) { }

	inline ~obj_t() throw() { }

	obj_t(obj_t const &) = delete;
	obj_t &operator=(obj_t const &) = delete;

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

void obj_init(obj_t *obj = obj_t::list);
void obj_exec(obj_t const *obj = obj_t::list);
void obj_stat_print(obj_t const **objs, size_t size);
void obj_fini(obj_t *obj = obj_t::list);

obj_t const *obj_find(string_t const &name);
inline bool objs_exist() { return obj_t::list != NULL; }

size_t objs_all(obj_t const **objs, obj_t *obj = obj_t::list);

} // namespace phantom

#pragma GCC visibility pop
