// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/config_named_list.H>
#include <pd/base/config_syntax.H>

#pragma GCC visibility push(default)

namespace pd { namespace config {

template<typename val_t>
class enum_t {
	val_t val;
public:
	class item_t : public named_list_item_t<item_t> {
		using named_list_item_t<item_t>::next;
		using named_list_item_t<item_t>::name;

		val_t val;

		void print(out_t &out, val_t _val) {
			if(!this) {
				out(CSTR("<unknown enum value>"));
			}
			else if(val == _val) {
				out(name);
			}
			else
				next->print(out, _val);
		}

		void syntax(out_t &out, unsigned int &num) {
			if(!this) return;

			next->syntax(out, num);
			if(num) out(' ')('|')(' ');
			out(name);
			++num;
		}

	public:
		inline item_t(string_t const &name, val_t _val) :
			named_list_item_t<item_t>(name, this), val(_val) { }

		inline ~item_t() throw() { }

		friend class helper_t<enum_t<val_t> >;
	};

	inline enum_t(val_t _val) throw() : val(_val) { }
	inline ~enum_t() throw() { }

	inline operator val_t() const throw() { return val; }

	inline enum_t &operator=(val_t const &_val) throw() {
		val = _val; return *this;
	}

	static string_t sname;
};

#define config_enum_sname(val_t) \
	template<> \
	string_t config::enum_t<val_t>::sname(STRING("<" #val_t ">"));

#define config_enum_internal_sname(type_t, val_t) \
	template<> \
	string_t config::enum_t<type_t::val_t>::sname(STRING("<" #type_t "::" #val_t ">"));

#define config_enum_value(val_t, value) \
	static config::enum_t<val_t>::item_t config_enum_value_##val_t##_##value(STRING(#value), value)

#define config_enum_internal_value(type_t, val_t, value) \
	static config::enum_t<type_t::val_t>::item_t config_enum_value_##val_t##_##value(STRING(#value), type_t::value)

template<typename val_t>
struct helper_t<enum_t<val_t> > {
	static inline void parse(in_t::ptr_t &ptr, enum_t<val_t> &val) {
		val = enum_t<val_t>::item_t::find(parse_name(ptr), ptr).val;
	}

	static inline void print(out_t &out, int, enum_t<val_t> const &val) {
		enum_t<val_t>::item_t::list->print(out, val);
	}

	static inline void syntax_detail(out_t &out) {
		unsigned int num = 0;
		enum_t<val_t>::item_t::list->syntax(out, num);
	}

	static inline void syntax(out_t &out) {
		out(enum_t<val_t>::sname);
		syntax_t::push(enum_t<val_t>::sname, &syntax_detail);
	}
};

}} // namespace pd::config

#pragma GCC visibility pop
