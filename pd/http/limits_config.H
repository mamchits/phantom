// This file is part of the pd::http library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/http/http.H>

#include <pd/base/size.H>

#pragma GCC visibility push(default)

namespace pd { namespace http {

struct limits_t::config_t {
	sizeval_t line, field_num, field, entity;

	inline config_t(
		size_t _line, size_t _field_num, size_t _field, size_t _entity
	) throw() :
		line(_line), field_num(_field_num), field(_field), entity(_entity) { }

	inline void check(in_t::ptr_t const &) const { }

	inline ~config_t() throw() { }
};

}} // namespace pd::http

#pragma GCC visibility pop
