// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/config_block.H>
#include <pd/base/config_syntax.H>

#include <pd/base/list.H>

#pragma GCC visibility push(default)

namespace pd { namespace config {

template<typename val_t>
class record_t : public val_t, public block_t {
	class item_t : public list_item_t<item_t> {
		using list_item_t<item_t>::next;

		virtual void parse_item(in_t::ptr_t &ptr, val_t &val) const = 0;
		virtual void print_item(out_t &out, int off, val_t const &val) const = 0;
		virtual void syntax_item(out_t &out) const = 0;
	public:

		inline void parse(in_t::ptr_t &ptr, val_t &val) const {
			if(this) {
				next->parse(ptr, val);
				parse_item(ptr, val);
			}
		}

		inline void print(out_t &out, int off, val_t const &val) const {
			if(this) {
				next->print(out, off, val);
				print_item(out, off, val);
			}
		}

		inline void syntax(out_t &out) const {
			if(this) {
				next->syntax(out);
				syntax_item(out);
				out(' ');
			}
		}

		inline item_t() throw() : list_item_t<item_t>(this, list) { }
		inline ~item_t() throw() { }
	};

public:
	template<typename x_t>
	class value_t : public item_t {
		x_t val_t::*xp;

		virtual void parse_item(in_t::ptr_t &ptr, val_t &val) const {
			skip_space(ptr);
			helper_t<x_t>::parse(ptr, val.*xp);
		}

		virtual void print_item(out_t &out, int off, val_t const &val) const {
			print_off(out, off);
			helper_t<x_t>::print(out, off, val.*xp);
			out.lf();
		}

		virtual void syntax_item(out_t &out) const {
			helper_t<x_t>::syntax(out);
		}

	public:
		inline value_t(x_t val_t::*_xp) : item_t(), xp(_xp) { }
		inline ~value_t() throw() { }
	};

	inline record_t() : val_t(), block_t() { }
	inline ~record_t() throw() { }

	static inline void syntax(out_t &out) {
		out('{')(' ');
		list->syntax(out);
		out('}');
	}

	static string_t sname;

private:
	virtual void parse_content(in_t::ptr_t &ptr) {
		list->parse(ptr, *this);
		skip_space(ptr);
	}

	virtual void print_content(out_t &out, int off) const {
		list->print(out, off, *this);
	}

	static item_t *list;
};

template<typename val_t>
typename record_t<val_t>::item_t *record_t<val_t>::list = NULL;

template<typename val_t>
struct helper_t<record_t<val_t>> {
	static void parse(in_t::ptr_t &ptr, record_t<val_t> &val) {
		val.block_t::parse(ptr);
	}

	static void print(out_t &out, int off, record_t<val_t> const &val) {
		val.block_t::print(out, off);
	}

	static void syntax(out_t &out) {
		out(record_t<val_t>::sname);
		syntax_t::push(record_t<val_t>::sname, &record_t<val_t>::syntax);
	}
};

#define config_record_sname(val_t) \
	template<> \
	string_t config::record_t<val_t>::sname(STRING("<" #val_t ">"));

#define config_record_value(val_t, name) \
	static config::record_t<val_t>::value_t<typeof(((val_t *)NULL)->*(&val_t::name))> config_binding_value_rec_##name(&val_t::name);

}} // namespace pd::config

#pragma GCC visibility pop
