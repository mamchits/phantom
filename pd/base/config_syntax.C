// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "config_syntax.H"

namespace pd { namespace config {

syntax_t *syntax_t::list = NULL;

void syntax_t::proc(out_t &out) {
	for(syntax_t *item = list; item; item = item->next) {
		out(item->type_name)(' ')(':')('=')(' ');
		(*item->func)(out);
		out.lf().lf();
	}
}

void syntax_t::push(string_t const &type_name, void (*func)(out_t &)) {
	syntax_t **item = &list;
	for(; *item; item = &(*item)->next) {
		if(func == (*item)->func) return;
	}
	(*item) = new syntax_t(type_name, func);
}

void syntax_t::erase() {
	for(syntax_t *item = list; item;) {
		syntax_t *tmp = item;
		item = item->next;
		delete tmp;
	}
	list = NULL;
}

}} // namespace pd::config
