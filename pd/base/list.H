// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/thr.H>

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

private: // don't use
	list_item_t(list_item_t const &);
	list_item_t &operator=(list_item_t const &);
};

template<typename x_t>
class list_atomic_item_t {
	thr::spinlock_t &spin;

protected:
	x_t *next, **me;

	inline void unlink() {
		{
			thr::spinlock_guard_t _guard(spin);
			if((*me = next)) next->list_atomic_item_t<x_t>::me = me;
		}
		me = &next;
	}

	inline void link(x_t *_this, x_t *&list) {
		if(me != &next)
			unlink();

		thr::spinlock_guard_t _guard(spin);
		if((next = list)) next->list_atomic_item_t<x_t>::me = &next;
		*(me = &list) = _this;
	}

	inline list_atomic_item_t(thr::spinlock_t &_spin) : spin(_spin), me(&next) { }

	inline list_atomic_item_t(
		x_t *_this, x_t *&list, thr::spinlock_t &_spin
	) throw() : spin(_spin), me(&next) { link(_this, list); }

	inline ~list_atomic_item_t() throw() {
		if(me != &next)
			unlink();
	}

private: // don't use
	list_atomic_item_t(list_atomic_item_t const &);
	list_atomic_item_t &operator=(list_atomic_item_t const &);
};

} // namespace pd

#pragma GCC visibility pop
