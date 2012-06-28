// This file is part of the phantom::io_stream::proto_http::handler_static module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/string.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_stream { namespace proto_http { namespace handler_static {

struct file_type_t {
	string_t mime_type;
	bool need_charset;
	bool allow_gzip;

	inline file_type_t()
		: mime_type(), need_charset(false), allow_gzip(false) { }

	inline ~file_type_t() throw() { }
};

class file_types_t {
protected:
	struct item_t : public file_type_t {
		item_t *first;
		item_t *next;
		string_t ext;

		inline item_t() : file_type_t(), first(NULL), next(NULL), ext() { }
		inline ~item_t() throw() { }
	};

	item_t *items;
	size_t size;

	inline file_types_t() : items(NULL), size(0) { }
	inline ~file_types_t() throw() { if(items) delete [] items; }

	inline void init(size_t _size) {
		if(items)
			delete [] items;

		items = NULL; size = 0;

		if(_size)
			items = new item_t[size = _size];
	}

	inline item_t *&bucket(string_t const &ext) const {
		return items[ext.fnv<lower_t>() % size].first;
	}

	inline item_t *lookup_item(string_t const &ext) const {
		if(!size)
			return NULL;

		for(item_t *item = bucket(ext); item; item = item->next)
			if(string_t::cmp_eq<lower_t>(item->ext, ext))
				return item;

		return NULL;
	}

public:
	inline file_type_t const *lookup(string_t const &ext) const {
		return lookup_item(ext) ?: NULL;
	}

private:
	void *operator new(size_t);
	void operator delete(void *);
};

}}}} // namespace phantom::io_stream::proto_http::handler_static

#pragma GCC visibility pop
