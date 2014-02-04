// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/spinlock.H>

#pragma GCC visibility push(default)

namespace pd {

template<typename x_t>
class list_item_t {
protected:
	x_t *next, **me;

	inline list_item_t(x_t *_this, x_t *&list) throw() {
		if((next = list)) next->list_item_t<x_t>::me = &next;
		*(me = &list) = _this;
	}

	inline ~list_item_t() throw() {
		if((*me = next)) next->list_item_t<x_t>::me = me;
	}

	list_item_t(list_item_t const &) = delete;
	list_item_t &operator=(list_item_t const &) = delete;
};

} // namespace pd

#pragma GCC visibility pop
