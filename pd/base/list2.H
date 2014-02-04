// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#pragma GCC visibility push(default)

namespace pd {

template<typename x_t>
class list2_item_t {
protected:
	x_t *next, *prev;

	inline list2_item_t(x_t *_this) throw() {
		next = prev = _this;
	}

	inline ~list2_item_t() throw() {
		next->list2_item_t<x_t>::prev = prev;
		prev->list2_item_t<x_t>::next = next;
	}

	list2_item_t(list2_item_t const &) = delete;
	list2_item_t &operator=(list2_item_t const &) = delete;
};

} // namespace pd

#pragma GCC visibility pop
