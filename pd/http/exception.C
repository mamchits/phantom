// This file is part of the pd::http library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "http.H"

namespace pd { namespace http {

str_t exception_t::msg() const {
	return str_t(_msg, strlen(_msg));
}

void exception_t::log() const {
	log_put(log::error, NULL, "%u %s", _code, _msg);
}

exception_t::~exception_t() throw() { }

}} // namespace pd::http
