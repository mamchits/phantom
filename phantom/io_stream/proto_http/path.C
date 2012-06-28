// This file is part of the phantom::io_stream::proto_http module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "path.H"

namespace pd { namespace config {

typedef phantom::io_stream::proto_http::path_t path_t;

template<>
void helper_t<path_t>::parse(in_t::ptr_t &ptr, path_t &path) {
	helper_t<string_t>::parse(ptr, path);
	path.norm();
}

template<>
void helper_t<path_t>::print(out_t &out, int off, path_t const &path) {
	helper_t<string_t>::print(out, off, path);
}

template<>
void helper_t<path_t>::syntax(out_t &out) {
	helper_t<string_t>::syntax(out);
}

}} // namespace pd::config
