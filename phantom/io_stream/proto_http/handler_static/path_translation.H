// This file is part of the phantom::io_stream::proto_http::handler_static module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/string.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_stream { namespace proto_http { namespace handler_static {

class path_translation_t {
public:
	virtual string_t translate(
		string_t const &root, string_t const &path
	) const = 0;

protected:
	inline path_translation_t() throw() { }
	inline ~path_translation_t() throw() { }

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

}}}} // namespace phantom::io_stream::proto_http::handler_static

#pragma GCC visibility pop
