// This file is part of the phantom program.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/string.H>
#include <pd/base/list.H>

#pragma GCC visibility push(default)

#define MODULE_VERSION 0

namespace phantom {

class module_info_t : public list_item_t<module_info_t> {
	string_t name;
	unsigned long version;

	static module_info_t *list;
public:
	inline module_info_t(string_t const &_name) :
		list_item_t<module_info_t>(this, list),
		name(_name),
		version(MODULE_VERSION) { }

	static inline module_info_t *lookup(string_t const &name) {
		for(module_info_t *item = list; item; item = item->next)
			if(string_t::cmp_eq<ident_t>(name, item->name)) return item;

		return NULL;
	}

	inline ~module_info_t() throw() { }

	inline operator bool() const throw() { return version == MODULE_VERSION; }
};

void module_load(char const *file_name);

#define MODULE(name) \
	static phantom::module_info_t module_##name(STRING(#name))

} // namespace phantom

#pragma GCC visibility pop
