// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/config_helper.H>
#include <pd/base/list.H>

#pragma GCC visibility push(default)

namespace pd { namespace config {

template<typename x_t>
class named_list_item_t : public list_item_t<x_t> {
protected:
	string_t name;

	static x_t *list;

	inline named_list_item_t(string_t const &_name, x_t *_this) :
		list_item_t<x_t>(_this, list), name(_name) { }

	inline ~named_list_item_t() throw() { }

public:
	static inline x_t &find(string_t const &_name, in_t::ptr_t const &ptr) {
		for(x_t *item = list; item; item = item->next)
			if(string_t::cmp_eq<ident_t>(_name, item->name)) return *item;

		error(ptr, "name is not found");
	}

	static inline x_t *lookup(string_t const &_name) {
		for(x_t *item = list; item; item = item->next)
			if(_name == item->name) return item;

		return NULL;
	}

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

template<typename x_t>
x_t *named_list_item_t<x_t>::list = NULL;

}} // namespace pd::config

#pragma GCC visibility pop
