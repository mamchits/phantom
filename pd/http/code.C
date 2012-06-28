// This file is part of the pd::http library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "http.H"

namespace pd { namespace http {

string_t code_descr(code_t code) {
	switch(code) {
		case code_undefined: return string_t::empty;

#define HTTP_CODE(x, y) \
		case code_##x: return STRING(y);

		HTTP_CODE_LIST

#undef HTTP_CODE
	}
	return string_t::empty;
}

}} // namespace pd::http
