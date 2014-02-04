// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/list.H>
#include <pd/base/config_block.H>

#pragma GCC visibility push(default)

namespace pd { namespace config {

template<typename val_t>
class list_t : public block_t {
	class item_t : public list_item_t<item_t> {
		val_t val;

		inline item_t(item_t *&items) :
			list_item_t<item_t>(this, items), val() { }

		inline ~item_t() throw() { }

		friend class list_t;
		friend class list_t::ptr_t;
	} *items;

	inline void clear() { while(items) delete items; }

public:
	class ptr_t {
		item_t const *item;

	public:
		inline ptr_t(list_t const &l) throw() : item(l.items) { }
		inline operator bool() const throw() { return item != NULL; }

		inline ptr_t &operator++() throw() { item = item->next; return *this; }

		inline ptr_t operator++(int) throw() {
			ptr_t p = *this; item = item->next;
			return p;
		}

		inline val_t const &val() const throw() { return item->val; }
	};

	inline operator bool() const { return items != NULL; }

	inline ptr_t _ptr() const { return ptr_t(*this); }

	inline list_t() throw() : block_t(), items(NULL) { }
	inline ~list_t() throw() { clear(); }

	list_t(list_t const &) = delete;
	list_t &operator=(list_t const &) = delete;

private:
	virtual void parse_content(in_t::ptr_t &ptr) {
		clear();
		item_t **last = &items;

		while(skip_space(ptr)) {
			item_t *item = new item_t(*last);
			helper_t<val_t>::parse(ptr, item->val);
			last = &item->next;
		}
	}

	virtual void print_content(out_t &out, int off) const {
		for(item_t const *item = items; item; item = item->next) {
			print_off(out, off);
			helper_t<val_t>::print(out, off, item->val);
			out.lf();
		}
	}
};

template<typename val_t>
struct helper_t<list_t<val_t>> {
	static void parse(in_t::ptr_t &ptr, list_t<val_t> &val) {
		val.parse(ptr);
	}

	static void print(out_t &out, int off, list_t<val_t> const &val) {
		val.print(out, off);
	}

	static void syntax(out_t &out) {
		out('{')(' ')('[');
		helper_t<val_t>::syntax(out);
		out(' ')(']')('*')('}');
	}
};

} // namespace config

template<typename val_t>
class sarray1_t {
public:
	size_t size;
	val_t *items;

	template<typename _val_t>
	inline sarray1_t(
		config::list_t<_val_t> const &config_list
	) :
		size(0), items(NULL) {

		for(typeof(config_list._ptr()) ptr = config_list; ptr; ++ptr)
			++size;

		items = new val_t[size];

		try {
			val_t *p = items;

			for(typeof(config_list._ptr()) ptr = config_list; ptr; ++ptr)
				*(p++) = ptr.val();
		}
		catch(...) {
			delete [] items;
			throw;
		}
	}

	template<
		typename _val_t,
		typename cvt_t
			// Sorry, 4.6.3 ICE here
			/* = typeof([](_val_t const &_val) { return _val; } -> val_t) */
	>
	inline sarray1_t(
		config::list_t<_val_t> const &config_list,
		cvt_t cvt /* = [](_val_t const &_val) { return _val; } -> val_t */
	) :
		size(0), items(NULL) {

		for(typeof(config_list._ptr()) ptr = config_list; ptr; ++ptr)
			++size;

		items = new val_t[size];

		try {
			val_t *p = items;

			for(typeof(config_list._ptr()) ptr = config_list; ptr; ++ptr)
				*(p++) = cvt(ptr.val());
		}
		catch(...) {
			delete [] items;
			throw;
		}
	}

	inline ~sarray1_t() throw() { delete [] items; }

	sarray1_t(sarray1_t const &) = delete;
	sarray1_t &operator=(sarray1_t const &) = delete;

	inline val_t &operator[](size_t i) {
		assert(i < size);
		return items[i];
	}

	inline val_t const &operator[](size_t i) const {
		assert(i < size);
		return items[i];
	}
};

} // namespace pd

#pragma GCC visibility pop
