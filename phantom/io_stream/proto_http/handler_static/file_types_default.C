// This file is part of the phantom::io_stream::proto_http::handler_static module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "file_types.H"

#include <pd/base/config.H>
#include <pd/base/config_list.H>
#include <pd/base/string_file.H>

namespace phantom { namespace io_stream { namespace proto_http { namespace handler_static {

class file_types_default_t : public file_types_t {
	void parse_item(string_t const &_mime_type, in_t::ptr_t &p, size_t depth);

public:
	struct config_t {
		string_t mime_types_filename;
		config::list_t<string_t> allow_gzip_list;

		inline config_t() throw() :
			mime_types_filename(STRING("/etc/mime.types")), allow_gzip_list() { }

		inline void check(in_t::ptr_t const &) const { }
		inline ~config_t() throw() { }
	};

	file_types_default_t(string_t const &, config_t const &config);
	~file_types_default_t() throw();
};

namespace file_types_default {
config_binding_sname(file_types_default_t);
config_binding_value(file_types_default_t, mime_types_filename);
config_binding_value(file_types_default_t, allow_gzip_list);
config_binding_ctor(file_types_t, file_types_default_t);
}

// ugly function #1
static bool mime_type_need_charset(string_t const &mtype) {
	char const *p = mtype.ptr();

	return (
		mtype.size() >= 5 &&
		p[0] == 't' &&  p[1] == 'e' && p[2] == 'x' &&  p[3] == 't' && p[4] == '/'
	);
}

// ugly function #2
static bool mime_type_compare(string_t const &m1, string_t const &m2) {
	char const *b1 = m1.ptr();
	size_t m1_len = m1.size();
	char const *p1 = (char const *)memchr(b1, '/', m1_len);

	char const *b2 = m2.ptr();
	size_t m2_len = m2.size();
	char const *p2 = (char const *)memchr(b2, '/', m2_len);

	if(!p2) return true;
	if(!p1) return false;

	size_t l1 = p1 - b1;
	size_t l2 = p2 - b2;

	bool x1 = (l1 >= 2 && b1[0] == 'x' && b1[1] == '-');
	bool x2 = (l2 >= 2 && b2[0] == 'x' && b2[1] == '-');

	if(x2 && !x1) return true;
	if(!x2 && x1) return false;

	size_t k1 = m1_len - l1;
	size_t k2 = m2_len - l2;

	x1 = (k1 >= 3 && p1[1] == 'x' && p1[2] == '-');
	x2 = (k2 >= 3 && p2[1] == 'x' && p2[2] == '-');

	if(x2 && !x1) return true;
	if(!x2 && x1) return false;

	if(l1 < l2) return true;
	if(l2 < l1) return false;

	if(m1_len < m2_len) return true;
	return false;
}

static inline bool is_hspace(char c) {
	return (c == ' ' || c == '\t');
}

static inline bool is_eol(char c) {
	return (c == '\n' || c == '\0');
}

void file_types_default_t::parse_item(
	string_t const &_mime_type, in_t::ptr_t &p, size_t depth
) {
	string_t mime_type = _mime_type;

	if(!mime_type) {
		while(true) { // skip comments
			if(!p) {
				init(depth);
				return;
			}

			if(*p != '#')
				break;

			++p;
			while(!is_eol(*p)) ++p;
			++p;
		}

		in_t::ptr_t p0 = p;

		while(!is_hspace(*p) && !is_eol(*p)) ++p;

		mime_type = string_t(p0, p - p0);
	}

	while(is_hspace(*p)) ++p;

	if(is_eol(*p)) {
		++p;
		parse_item(string_t::empty, p, depth);
		return;
	}

	in_t::ptr_t p0 = p;

	while(!is_hspace(*p) && !is_eol(*p)) ++p;

	string_t ext = string_t(p0, p - p0);

	parse_item(mime_type, p, depth + 1);

	item_t &item = items[depth];
	item.mime_type = mime_type;
	item.ext = ext;
	item.need_charset = mime_type_need_charset(mime_type);

	item_t *&list = bucket(ext);

	for(item_t **itempp = &list; *itempp; itempp = &(*itempp)->next) {
		item_t *itemp = *itempp;

		if(string_t::cmp<lower_t>(item.ext, itemp->ext)) {
			if(mime_type_compare(item.mime_type, itemp->mime_type)) {
				*itempp = itemp->next;
				itemp->next = NULL;
				break;
			}
			else
				return;
		}
	}

	item.next = list;
	list = &item;

	return;
}

file_types_default_t::file_types_default_t(string_t const &, config_t const &config) :
	file_types_t() {

	if(config::check)
		return;

	string_t string = string_file(config.mime_types_filename.str());
	in_t::ptr_t p = string;
	parse_item(string_t::empty, p, 0);

	if(!size)
		return;

	for(
		config::list_t<string_t>::ptr_t ptr = config.allow_gzip_list;
		ptr; ++ptr
	) {
		item_t *item = lookup_item(ptr.val());

		if(item) item->allow_gzip = true;
	}
}

file_types_default_t::~file_types_default_t() throw() { }

}}}} // namespace phantom::io_stream::proto_http::handler_static
