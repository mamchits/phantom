// This file is part of the phantom::io_stream::proto_http::handler_static module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "file_types.H"

#include <pd/base/config_switch.H>
#include <pd/base/config_struct.H>
#include <pd/base/config_enum.H>

namespace phantom { namespace io_stream { namespace proto_http { namespace handler_static {

class file_types_list_t : public file_types_t {
public:
	struct type_config_t {
		string_t mime_type;
		config::enum_t<bool> need_charset;
		config::enum_t<bool> allow_gzip;

		inline type_config_t() throw() :
			mime_type(), need_charset(false), allow_gzip(false) { }

		inline void check(in_t::ptr_t const &) const { }

		inline ~type_config_t() throw() { }
	};

	struct config_t {
		config::switch_t<string_t, config::struct_t<type_config_t>, string_t::cmp<lower_t>> list;

		inline config_t() throw() : list() { }
		inline void check(in_t::ptr_t const &) const { }
		inline ~config_t() throw() { }
	};

	file_types_list_t(string_t const &, config_t const &config);
	~file_types_list_t() throw();
};

namespace file_types_list {
config_binding_sname_sub(file_types_list_t, type);
config_binding_value_sub(file_types_list_t, type, mime_type);
config_binding_value_sub(file_types_list_t, type, need_charset);
config_binding_value_sub(file_types_list_t, type, allow_gzip);

config_binding_sname(file_types_list_t);
config_binding_value(file_types_list_t, list);
config_binding_cast(file_types_list_t, file_types_t);
config_binding_ctor(file_types_t, file_types_list_t);
}

file_types_list_t::file_types_list_t(string_t const &, config_t const &config) :
	file_types_t() {

	size_t size = 0;

	for(typeof(config.list._ptr()) ptr = config.list; ptr; ++ptr)
		++size;

	if(!size)
		return;

	init(size);

	item_t *iptr = items;

	for(typeof(config.list._ptr()) ptr = config.list; ptr; ++ptr) {
		string_t const &ext = ptr.key();
		type_config_t const &config = ptr.val();

		iptr->ext = ext;
		iptr->mime_type = config.mime_type;
		iptr->need_charset = config.need_charset;
		iptr->allow_gzip = config.allow_gzip;

		item_t *&list = bucket(ext);
		iptr->next = list;
		list = iptr;

		++iptr;
	}
}

file_types_list_t::~file_types_list_t() throw() { }

}}}} // namespace phantom::io_stream::proto_http::handler_static
