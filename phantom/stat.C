// This file is part of the phantom program.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "stat.H"
#include "setup.H"

#include <pd/base/list.H>
#include <pd/base/stat.H>
#include <pd/base/config.H>
#include <pd/base/config_list.H>

namespace phantom {

struct stat_label_t : list_item_t<stat_label_t> {
	static stat_label_t *list;

	string_t const name;

	inline stat_label_t(string_t const &_name) :
		list_item_t<stat_label_t>(this, list), name(_name.copy()) { }

	inline ~stat_label_t() throw() { }

/*
	static inline size_t count() {
		size_t count = 0;

		for(stat_label_t *ptr = stat_label_t::list; ptr; ptr = ptr->next)
			++count;

		return count;
	}
*/

	static inline void create(string_t const &_name) {
		new stat_label_t(_name);
		++stat::res_count;
	}

	friend class pd::config::helper_t<stat_id_t>;

	struct cleanup_t {
		inline cleanup_t() throw() { }
		inline ~cleanup_t() throw() { while(list) delete list; }
	};
};

stat_label_t *stat_label_t::list = NULL;

static stat_label_t::cleanup_t cleanup;

//size_t stat_res_count() { return stat_label_t::count(); }

class setup_stat_t : public setup_t {
public:
	struct name_t : public string_t {
		inline name_t() : string_t() { }
		inline name_t(string_t const &str) : string_t(str) { }
		inline ~name_t() throw() { }
	};

	struct config_t {
		config::list_t<name_t> list;

		inline config_t() throw() : list() { }
		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &) const { }
	};

	inline setup_stat_t(string_t const &, config_t const &config) throw() {
		for(typeof(config.list._ptr()) ptr = config.list; ptr; ++ptr)
			stat_label_t::create(ptr.val());
	}
};

namespace setup_stat {
config_binding_sname(setup_stat_t);
config_binding_value(setup_stat_t, list);
config_binding_ctor(setup_t, setup_stat_t);
}

} // namespace phantom

namespace pd { namespace config {

typedef phantom::stat_id_t stat_id_t;
typedef phantom::stat_label_t stat_label_t;

template<>
void helper_t<stat_id_t>::parse(in_t::ptr_t &ptr, stat_id_t &val) {
	string_t const name = parse_name(ptr);

	size_t count = 0;

	for(stat_label_t *ptr = stat_label_t::list; ptr; ptr = ptr->next) {
		if(string_t::cmp_eq<ident_t>(name, ptr->name)) {
			val.res_no = count;
			return;
		}

		++count;
	}

	error(ptr, "name not found");
}

template<>
void helper_t<stat_id_t>::print(out_t &out, int, stat_id_t const &val) {
	size_t count = val.res_no;

	for(stat_label_t *ptr = stat_label_t::list; ptr; ptr = ptr->next) {
		if(!count) {
			out(ptr->name);
			return;
		}
		--count;
	}

	out(CSTR("<unknown stat counter>"));
}

template<>
void helper_t<stat_id_t>::syntax(out_t &out) {
	out(CSTR("<stat_name>"));
}

typedef phantom::setup_stat_t::name_t name_t;

template<>
void helper_t<name_t>::parse(in_t::ptr_t &ptr, name_t &name) {
	name = name_t(parse_name(ptr));
}

template<>
void helper_t<name_t>::print(out_t &out, int, name_t const &name) {
	out(name);
}

template<>
void helper_t<name_t>::syntax(out_t &out) {
	out(CSTR("<stat_name>"));
}

}} // namespace pd::config
