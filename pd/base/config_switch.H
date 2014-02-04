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

template<typename key_t, typename val_t, cmp_t cmp_func(key_t const &, key_t const &) = key_t::cmp>
class switch_t : public block_t {
	class item_t : public list_item_t<item_t> {
		key_t key;
		val_t val;

		inline item_t(key_t const &_key, item_t *&items) :
			list_item_t<item_t>(this, items), key(_key), val() { }

		inline ~item_t() throw() { }

		friend class switch_t;
		friend class switch_t::ptr_t;
	} *items;

	inline val_t &find(key_t const &key) {
		item_t **item;
		for(item = &items; *item; item = &(*item)->next) {
			cmp_t res = cmp_func(key, (*item)->key);
			if(res) return (*item)->val;
			if(res.is_less()) break;
		}

		return (new item_t(key, *item))->val;
	}

	inline void clear() { while(items) delete items; }

public:
	class ptr_t {
		item_t const *item;

	public:
		inline ptr_t(switch_t const &s) throw() : item(s.items) { }
		inline operator bool() const throw() { return item != NULL; }

		inline ptr_t &operator++() throw() { item = item->next; return *this; }

		inline ptr_t operator++(int) throw() {
			ptr_t p = *this; item = item->next;
			return p;
		}

		inline key_t const &key() const throw() { return item->key; }
		inline val_t const &val() const throw() { return item->val; }
	};

	inline operator bool() const { return items != NULL; }

	inline ptr_t _ptr() const { return ptr_t(*this); }

	inline switch_t() throw() : block_t(), items(NULL) { }
	inline ~switch_t() throw() { clear(); }

	switch_t(switch_t const &) = delete;
	switch_t &operator=(switch_t const &) = delete;

private:
	virtual void parse_content(in_t::ptr_t &ptr) {
		clear();

		while(skip_space(ptr)) {
			key_t key;
			helper_t<key_t>::parse(ptr, key);

			if(skip_space(ptr) != ':')
				error(ptr, "':' is expected");

			if(!skip_space(++ptr))
				error(ptr, "value is expected");

			helper_t<val_t>::parse(ptr, find(key));
		}
	}

	virtual void print_content(out_t &out, int off) const {
		for(item_t *item = items; item; item = item->next) {
			print_off(out, off);
			helper_t<key_t>::print(out, off, item->key);
			out(CSTR(" : "));
			helper_t<val_t>::print(out, off, item->val);
			out.lf();
		}
	}
};

template<typename key_t, typename val_t, cmp_t cmp_func(key_t const &, key_t const &)>
struct helper_t<switch_t<key_t, val_t, cmp_func>> {
	static void parse(in_t::ptr_t &ptr, switch_t<key_t, val_t, cmp_func> &val) {
		val.parse(ptr);
	}

	static void print(out_t &out, int off, switch_t<key_t, val_t, cmp_func> const &val) {
		val.print(out, off);
	}

	static void syntax(out_t &out) {
		out('{')(' ')('[');
		helper_t<key_t>::syntax(out);
		out(' ')(':')(' ');
		helper_t<val_t>::syntax(out);
		out(' ')(']')('*')('}');
	}
};

} // namespace config

template<typename key_t, typename val_t>
class darray2_t {
protected:
	struct item_t {
		key_t key;
		val_t val;

		template<typename _key_t, typename _val_t>
		inline item_t(_key_t const &_key, _val_t const &_val) :
			key(_key), val(_val) { }

		inline ~item_t() throw() { }
	};

	size_t size;
	item_t **items;

	template<typename _key_t, typename _val_t, cmp_t cmp_func(_key_t const &, _key_t const &)>
	inline darray2_t(
		config::switch_t<_key_t, _val_t, cmp_func> const &config_switch
	) :
		size(0), items(NULL) {

		for(typeof(config_switch._ptr()) ptr = config_switch; ptr; ++ptr)
			++size;

		items = new item_t *[size];
		item_t **p = items;

		try {
			for(typeof(config_switch._ptr()) ptr = config_switch; ptr; ++ptr)
				*(p++) = new item_t(ptr.key(), ptr.val());
		}
		catch(...) {
			while(--p >= items)
				delete *p;

			delete [] items;

			throw;
		}
	}

	inline ~darray2_t() throw() {
		for(size_t i = 0; i < size; ++i)
			delete items[i];

		delete [] items;
	}

	darray2_t(darray2_t const &) = delete;
	darray2_t &operator=(darray2_t const &) = delete;
};

} // namespace pd

#pragma GCC visibility pop
