// This file is part of the pd::http library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "client.H"

namespace pd { namespace http {

bool remote_reply_t::parse(
	in_t::ptr_t &ptr, limits_t const &limits, bool header_only
) {
	in_t::ptr_t ptr0 = ptr;

	try {
		if(!ptr)
			throw exception_t(code_400, "Empty response");

		version = parse_version(ptr);

		if(version == version_undefined || *(ptr++) != ' ')
			throw exception_t(code_400, "Not a HTTP reply (#1)");

		code = ({
			unsigned int _code = 0;
			for(int i = 0; i < 3; ++i) {
				char c = *(ptr++);
				if(c < '0' || c > '9')
					throw exception_t(code_400, "Not a HTTP reply (#2)");

				_code = _code * 10 + (c - '0');
			}

			if(_code < 100 || *(ptr++) != ' ')
				throw exception_t(code_400, "Not a HTTP reply (#3)");

			(code_t)_code;
		});

		eol_t eol;

		{
			in_t::ptr_t p0 = ptr;
			size_t limit = limits.line;

			if(!ptr.scan("\r\n", 2, limit))
				throw exception_t(code_400, "Not a HTTP reply (#4)");

			status_line = in_segment_t(p0, ptr - p0);
		}

		if(!eol.set(ptr))
			throw exception_t(code_400, "Not a HTTP reply (#4)");

		header.parse(ptr, eol, limits);

		bool res =
			header_only || code < code_200 || code == code_204 || code == code_304
			? true
			: entity.parse(ptr, eol, header, limits, true)
		;

		all = in_segment_t(ptr0, ptr - ptr0);

		return res;
	}
	catch(http::exception_t const &) {
		ptr.seek_end();

		all = in_segment_t(ptr0, ptr - ptr0);

		throw;
	}
}

}} // namespace pd::http
