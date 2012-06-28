// This file is part of the phantom::io_stream::proto_http module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/config_switch.H>
#include <pd/base/config_struct.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_stream { namespace proto_http {

class path_t : public string_t {
public:
	inline path_t() throw() : string_t() { }
	inline ~path_t() throw() { }

	inline void norm() throw() {
		char const *p = ptr();
		while(off_end > off_begin && p[off_end - off_begin - 1] == '/') --off_end;
	}

	inline bool match(string_t const &str, size_t &priv, bool &cont) {
		size_t res;
		in_t::ptr_t s = str;
		char const *p = ptr();
		size_t len = size();

		for(res = 0; s && res < len; ++res)
			if(*(p++) != *(s++)) break;

		if(res < priv)
			cont = false;
		else
			priv = res;

		return (res == len && (!s || *s == '/'));
	}

	static inline cmp_t cmp(path_t const &p1, path_t const &p2) {
		return string_t::cmp<ident_t>(p1, p2);
	}
};

template<typename data_t>
class paths_t {
public:
	struct item_t {
		path_t path;
		data_t data;

	private:
		inline item_t() throw() : path(), data() { }
		inline ~item_t() throw() { }

		template<typename _data_t>
		inline void set(path_t const &_path, _data_t const &_data) {
			path = _path; data = _data;
		}

		friend class paths_t;
	};

protected:
	size_t size;
	item_t *items;

public:
	inline paths_t() throw() : size(0), items(NULL) { }
	inline ~paths_t() throw() { if(items) delete [] items; }

	template<typename _data_t>
	inline void setup(
		config::switch_t<path_t, config::struct_t<_data_t> > const &config_switch
	) {
		if(items) delete [] items;
		size = 0;

		for(typename config::switch_t<path_t, config::struct_t<_data_t> >::ptr_t pptr = config_switch; pptr; ++pptr)
			++size;

		items = new item_t[size];

		item_t *item = items;
		for(typename config::switch_t<path_t, config::struct_t<_data_t> >::ptr_t pptr = config_switch; pptr; ++pptr)
			(item++)->set(pptr.key(), pptr.val());
	}

	inline item_t *lookup(string_t const &str) const {
		size_t priv = 0;
		bool cont = true;
		item_t *res = NULL;

		// TODO: Make nonlinear search.

		for(size_t i = 0; i < size; ++i) {
			if(items[i].path.match(str, priv, cont)) res = &items[i];
			if(!cont) break;
		}

		return res;
	}
};

}}} // namespace phantom::io_stream::proto_http

#pragma GCC visibility pop
