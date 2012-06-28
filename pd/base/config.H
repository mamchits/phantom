// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/config_named_list.H>
#include <pd/base/config_block.H>
#include <pd/base/config_syntax.H>

#pragma GCC visibility push(default)

namespace pd { namespace config {

template<typename x_t>
struct objptr_t {
	x_t *ptr;

	inline objptr_t() throw() : ptr(NULL) { }
	inline ~objptr_t() throw() { }

	operator x_t *() const { return ptr; }
	x_t *operator ->() const { return ptr; }
};

template<typename x_t>
class obj_t : public named_list_item_t<obj_t<x_t> > {
	x_t *obj;

public:
	inline obj_t(string_t const &name, x_t *_obj) :
		named_list_item_t<obj_t<x_t> >(name, this), obj(_obj) { }

	inline ~obj_t() throw() { }

	static string_t sname;

	friend class helper_t<objptr_t<x_t> >;
};

template<typename x_t>
class obj_ref_t {
public:
	class item_t : public list_item_t<item_t>, protected block_t {
		using list_item_t<item_t>::next;

		string_t name;
		string_t ctor_name;

		void print(out_t &out, int off, string_t const &type_name) const {
			if(!this) return;
			next->print(out, off, type_name);

			print_off(out, off);
			out(type_name)(' ')(name)(' ')('=')(' ')(ctor_name)(' ');

			block_t::print(out, off);

			out.lf();
		}

	protected:
		inline item_t(
			string_t const &_name, string_t const &_ctor_name, obj_ref_t &ref
		) :
			list_item_t<item_t>(this, ref.list), block_t(), name(_name),
			ctor_name(_ctor_name) { }

		virtual ~item_t() throw() { }

		friend class obj_ref_t;
	};

private:
	item_t *list;

	friend class item_t;

public:
	inline obj_ref_t() throw() : list(NULL) { }
	inline ~obj_ref_t() throw() { while(list) delete list; }

	inline void print(out_t &out, int off, string_t const &type_name) const {
		list->print(out, off, type_name);
	}
};

template<typename x_t>
class ctor_base_t : public named_list_item_t<ctor_base_t<x_t> > {
	using named_list_item_t<ctor_base_t<x_t> >::list;
	using named_list_item_t<ctor_base_t<x_t> >::next;

protected:
	inline ctor_base_t(string_t const &name) :
		named_list_item_t<ctor_base_t>(name, this) { }

	inline ~ctor_base_t() throw() { }

	virtual void syntax_object_item(out_t &out) const = 0;

	void syntax_object(out_t &out, unsigned int &num) {
		if(!this) return;
		next->syntax_object(out, num);

		if(num) out(' ')('|');
		out.lf()(' ')(' ');
		syntax_object_item(out);
		++num;
	}

	virtual void syntax_name_item(out_t &out) const = 0;

	void syntax_name(out_t &out, unsigned int &num) {
		if(!this) return;
		next->syntax_name(out, num);

		if(num) out(' ')('|');
		out.lf()(' ')(' ');
		syntax_name_item(out);
		++num;
	}

public:
	virtual void create(
		string_t const &name, in_t::ptr_t &ptr, obj_ref_t<x_t> &ref
	) = 0;

	static inline void syntax_object(out_t &out) {
		unsigned int num = 0;
		list->syntax_object(out, num);
	}

	static inline void syntax_name(out_t &out) {
		unsigned int num = 0;
		list->syntax_name(out, num);
	}
};

class value_parser_base_t {
	virtual void parse_value(in_t::ptr_t &ptr) = 0;

protected:
	inline value_parser_base_t() throw() { }
	inline ~value_parser_base_t() throw() { }

public:
	void parse(in_t::ptr_t &ptr);

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

template<typename val_t>
class value_parser_t : public value_parser_base_t {
	val_t &val;

	virtual void parse_value(in_t::ptr_t &ptr) { helper_t<val_t>::parse(ptr, val); }

public:
	inline value_parser_t(val_t &_val) : val(_val) { }
};

template<typename conf_t>
class binding_t {
	class item_t : public list_item_t<item_t> {
		using list_item_t<item_t>::next;

		virtual bool parse_item(
			string_t const &name, in_t::ptr_t &ptr, conf_t &conf
		) const = 0;

		bool parse(string_t const &name, in_t::ptr_t &ptr, conf_t &conf) const {
			return
				this && (
					next->parse(name, ptr, conf) || parse_item(name, ptr, conf)
				)
			;
		}

		virtual void print_item(out_t &out, int off, conf_t const &conf) const = 0;

		virtual void syntax_item(out_t &out, unsigned int &num) const = 0;

		void print(out_t &out, int off, conf_t const &conf) const {
			if(!this) return;
			next->print(out, off, conf);
			print_item(out, off, conf);
		}

		void syntax(out_t &out, unsigned int &num) const {
			if(!this) return;
			next->syntax(out, num);
			syntax_item(out, num);
		}

	public:
		inline item_t() throw() : list_item_t<item_t>(this, list) { }
		inline ~item_t() throw() { }

		friend class binding_t<conf_t>;
	};

	static item_t *list;

public:
	static inline bool parse_item(
		string_t const &name, in_t::ptr_t &ptr, conf_t &conf
	) {
		return list->parse(name, ptr, conf);
	}

	static inline void check(in_t::ptr_t const &ptr, conf_t const &conf) {
		conf.check(ptr);
	}

	static inline void parse(in_t::ptr_t &ptr, conf_t &conf) {
		in_t::ptr_t p = ptr;

		while(skip_space(p)) {
			string_t name = parse_name(p);

			if(!parse_item(name, p, conf))
				error(p, "unknown name");
		}

		ptr = p;

		check(ptr, conf);

		return;
	}

	static inline void print(out_t &out, int off, conf_t const &conf) {
		list->print(out, off, conf);
	}

	static inline void syntax(out_t &out) {
		unsigned int num = 0;
		list->syntax(out, num);
	}

	template<typename val_t>
	class value_t : public item_t {
		string_t name;
		val_t conf_t::*valp;

		virtual bool parse_item(
			string_t const &_name, in_t::ptr_t &ptr, conf_t &conf
		) const {
			if(!string_t::cmp_eq<ident_t>(name, _name)) return false;

			value_parser_t<val_t>(conf.*valp).parse(ptr);

			return true;
		}

		virtual void print_item(out_t &out, int off, conf_t const &conf) const {
			print_off(out, off);
			out(name)(CSTR(" = "));
			helper_t<val_t>::print(out, off, conf.*valp);
			out.lf();
		}

		virtual void syntax_item(out_t &out, unsigned int &num) const {
			if(num) out(' ')('|');
			out.lf()(' ')(' ')(name)(CSTR(" = "));
			helper_t<val_t>::syntax(out);
			++num;
		}

	public:
		inline value_t(string_t const &_name, val_t conf_t::*_valp) :
			item_t(), name(_name), valp(_valp) { }

		inline ~value_t() throw() { }
	};

	template<typename x_t>
	class type_t : public item_t {
		string_t name;
		string_t sname;
		obj_ref_t<x_t> conf_t::*refp;

		virtual bool parse_item(
			string_t const &_name, in_t::ptr_t &ptr, conf_t &conf
		) const {
			if(!string_t::cmp_eq<ident_t>(name, _name)) return false;

			in_t::ptr_t p = ptr;

			if(!skip_space(p))
				error(p, "object name is expected");

			string_t const &obj_name = parse_name(p);

			if(skip_space(p) != '=')
				error(p, "'=' is expected");

			if(!skip_space(++p))
				error(p, "constructor name is expected");

			string_t const &ctor_name = parse_name(p);

			ctor_base_t<x_t>::find(ctor_name, p).create(obj_name, p, conf.*refp);

			ptr = p;

			return true;
		}

		virtual void print_item(out_t &out, int off, conf_t const &conf) const {
			(conf.*refp).print(out, off, name);
		}

		virtual void syntax_item(out_t &out, unsigned int &num) const {
			if(num) out(' ')('|');
			out.lf()(' ')(' ')(name)(' ')(sname);

			syntax_t::push(sname, &ctor_base_t<x_t>::syntax_object);

			++num;
		}

	public:
		inline type_t(
			string_t const &_name, string_t const &_sname,
			obj_ref_t<x_t> conf_t::*_refp
		) :
			item_t(), name(_name), sname(_sname), refp(_refp) { }

		inline ~type_t() throw() { }
	};

	template<typename parent_conf_t>
	class parent_t : public item_t {
		virtual bool parse_item(
			string_t const &_name, in_t::ptr_t &ptr, conf_t &conf
		) const {
			return binding_t<parent_conf_t>::parse_item(_name, ptr, conf);
		}

		virtual void print_item(out_t &out, int off, conf_t const &conf) const {
			binding_t<parent_conf_t>::print(out, off, conf);
		}

		virtual void syntax_item(out_t &out, unsigned int &num) const {
			if(num) out(' ')('|');
			out.lf()(' ')(' ')(binding_t<parent_conf_t>::sname);

			syntax_t::push(
				binding_t<parent_conf_t>::sname,
				&binding_t<parent_conf_t>::syntax
			);

			++num;
		}

	public:
		inline parent_t() throw() : item_t() { }
		inline ~parent_t() throw() { }
	};

	class removed_t : public item_t {
		string_t name;
		char const *message;

		virtual bool parse_item(
			string_t const &_name, in_t::ptr_t &ptr, conf_t &
		) const {
			if(!string_t::cmp_eq<ident_t>(name, _name)) return false;

			error(ptr, message);
		}

		virtual void print_item(out_t &, int, conf_t const &) const { }
		virtual void syntax_item(out_t &out, unsigned int &) const { }

	public:
		inline removed_t(string_t const &_name, char const *_message) throw() :
			item_t(), name(_name), message(_message) { }

		inline ~removed_t() throw() { }
	};

	static string_t sname;
};

template<typename conf_t>
typename binding_t<conf_t>::item_t *binding_t<conf_t>::list = NULL;

#define config_binding_sname(x_t) \
	template <> \
	string_t config::binding_t<x_t::config_t>::sname(STRING("<" #x_t "::config>"))

#define config_binding_sname_sub(x_t, pref) \
	template <> \
	string_t config::binding_t<x_t::pref##_config_t>::sname(STRING("<" #x_t "::" #pref "_config>"))

#define config_binding_value_(x_t, config_t, name) \
static config::binding_t<x_t::config_t>::value_t<typeof(((x_t::config_t *)NULL)->*(&x_t::config_t::name))> \
	config_binding_value_##name(STRING(#name), &x_t::config_t::name)

#define config_binding_value(x_t, name) \
	config_binding_value_(x_t, config_t, name)

#define config_binding_value_sub(x_t, pref, name) \
	config_binding_value_(x_t, pref##_config_t, name)

#define config_binding_type_ref(y_t) \
	config::obj_ref_t<y_t> y_t##_ref

#define config_binding_type(x_t, y_t) \
template<> \
string_t config::obj_t<x_t::y_t>::sname = STRING("<" #x_t "::" #y_t "::name>"); \
static config::binding_t<x_t::config_t>::type_t<x_t::y_t> \
	config_binding_type_##y_t(STRING(#y_t), STRING("<" #x_t "::" #y_t "::object>"), &x_t::config_t::y_t##_ref)

#define config_binding_parent(x_t, x_parent_t, n) \
static config::binding_t<x_t::config_t>::parent_t<x_parent_t::config_t> \
	config_binding_parent_##n

#define config_binding_removed(x_t, name, message) \
static config::binding_t<x_t::config_t>::removed_t \
	config_binding_removed_##name(STRING(#name), message)

template<typename x_t, typename real_x_t>
class ctor_t : public ctor_base_t<x_t> {
	typedef typename real_x_t::config_t config_t;
	using ctor_base_t<x_t>::name;

	class obj_item_t :
		public list_item_t<obj_item_t>, public obj_ref_t<x_t>::item_t {

		// typedef typename real_x_t::config_t config_t;

		config_t config;
		real_x_t obj;
		obj_t<x_t> obj_item;
		obj_t<real_x_t> obj_item_real;

		virtual void parse_content(in_t::ptr_t &ptr) {
			binding_t<config_t>::parse(ptr, config);
		}

		virtual void print_content(out_t &out, int off) const {
			binding_t<config_t>::print(out, off, config);
		}

		inline config_t const &parse_conf(in_t::ptr_t &ptr) {
			block_t::parse(ptr);
			return config;
		}

	public:
		inline obj_item_t(
			string_t const &name, string_t const &ctor_name,
			obj_item_t *&list, obj_ref_t<x_t> &ref, in_t::ptr_t &ptr
		) :
			list_item_t<obj_item_t>(this, list),
			obj_ref_t<x_t>::item_t(name, ctor_name, ref),
			config(),
			obj(name, parse_conf(ptr)),
			obj_item(name, &obj),
			obj_item_real(name, &obj) { }

		virtual ~obj_item_t() throw() { }

	private: // don't use
		obj_item_t(obj_item_t const &);
		obj_item_t &operator=(obj_item_t const &);
	} *list;

	virtual void syntax_object_item(out_t &out) const {
		out('<')(name)(CSTR("::name> = "))(name)(' ')('{')(' ')('[')
			(binding_t<config_t>::sname)(' ')(']')('*')('}');

		syntax_t::push(binding_t<config_t>::sname, &binding_t<config_t>::syntax);
	}

	virtual void syntax_name_item(out_t &out) const {
		out('<')(name)(CSTR("::name>"));
	}

public:
	inline ctor_t(string_t const &name) : ctor_base_t<x_t>(name), list(NULL) { }

	inline ~ctor_t() throw() { while(list) delete list; }

	virtual void create(
		string_t const &obj_name, in_t::ptr_t &ptr, obj_ref_t<x_t> &ref
	) {
		try {
			new obj_item_t(obj_name.copy(), name, list, ref, ptr);
		}
		catch(exception_t const &) {
			report_obj(obj_name);
			throw;
		}
	}
};

#define config_binding_ctor(x_t, real_x_t) \
template<> \
string_t config::obj_t<real_x_t>::sname = STRING("<" #real_x_t "::name>"); \
static config::ctor_t<x_t, real_x_t> config_ctor_##real_x_t(STRING(#real_x_t))

#define config_binding_ctor_(x_t) \
static config::ctor_t<x_t, x_t> config_ctor_##x_t(STRING(#x_t))

template<typename x_t>
struct helper_t<objptr_t<x_t> > {
	static inline void parse(in_t::ptr_t &ptr, objptr_t<x_t> &val) {
		string_t name = parse_name(ptr);
		val.ptr = string_t::cmp_eq<ident_t>(name, CSTR("NULL"))
			? NULL
			: obj_t<x_t>::find(name, ptr).obj
		;
	}

	static inline void print(out_t &out, int, objptr_t<x_t> const &val) {
		if(val) {
			for(obj_t<x_t> *item = obj_t<x_t>::list; item; item = item->next) {
				if(val == item->obj) {
					out(item->name);
					return;
				}
			}

			out(CSTR("<unknown object>"));
		}
		else
			out(CSTR("NULL"));
	}

	static inline void syntax_detail(out_t &out) {
		out(CSTR("<name>"));
	}

	static inline void syntax(out_t &out) {
		out(obj_t<x_t>::sname);
		syntax_t::push(obj_t<x_t>::sname, &ctor_base_t<x_t>::syntax_name);
	}
};

}} // namespace pd::config

#pragma GCC visibility pop
