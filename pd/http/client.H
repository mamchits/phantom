// This file is part of the pd::http library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/http/http.H>

#pragma GCC visibility push(default)

namespace pd { namespace http {

class remote_reply_t {
public:
	version_t version;
	code_t code;
	in_segment_t status_line;
	mime_header_t header;
	entity_t entity;
	in_segment_t all;

	inline remote_reply_t() throw() :
		version(version_undefined), code(code_undefined),
		status_line(), header(), entity(), all() { }

	inline ~remote_reply_t() throw() { }

	inline void clear() {
		version = version_undefined;
		code = code_undefined;
		status_line.clear();
		header.clear();
		entity.clear();
		all.clear();
	}

	bool parse(in_t::ptr_t &ptr, limits_t const &limits, bool header_only);

private: // don't use
	remote_reply_t(remote_reply_t const &);
	remote_reply_t &operator=(remote_reply_t const &);
};

}} // namespace pd::http

#pragma GCC visibility pop
