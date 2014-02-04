// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "str.H"
#include "out.H"

namespace pd {

static inline unsigned short __ucs2(str_t::ptr_t &ptr) {
	str_t::ptr_t p = ptr;
	unsigned char c = *(p++);
	unsigned short l;

	if (c < 0x80) {
		l = c;
	}
	else if (c < 0xc0) {
		goto out;
	}
	else if (c < 0xe0) {
		if(!p) goto out;
		l = (c & 0x1f) << 6;
		if (((c = *(p++)) & 0xc0) == 0x80) {
			l |= (c & 0x3f);
		}
		else goto out;
	}
	else if (c < 0xf0) {
		if(!p) goto out;
		l = ((c & 0x0f) << 12);
		if (((c = *(p++)) & 0xc0) == 0x80) {
			if(!p) goto out;
			l |= ((c & 0x3f) << 6);
			if (((c = *(p++)) & 0xc0) == 0x80) {
				l |= (c & 0x3f);
			}
			else goto out;
		}
		else goto out;
	}
	else goto out;

	ptr = p;
	return l;

out:
	++ptr;
	return 0xfffd;
}

template<>
void out_t::helper_t<str_t>::print(
	out_t &out, str_t const &str, char const *fmt
) {
	enum { none, e, j } esc = none;
	bool b8 = false;

	if(fmt) {
		char c;
		while((c = *(fmt++))) {
			switch(c) {
				case 'e': esc = e; break;
				case 'j': esc = j; break;
				case '8': b8 = true; break;
			}
		}
	}

	switch(esc) {
		case e: {
			str_t::ptr_t ptr = str;

			out('"');

			while(ptr) {
				unsigned char c = *(ptr++);

				switch(c) {
					case '\r': c = 'r'; break;
					case '\n': c = 'n'; break;
					case '\t': c = 't'; break;
					case '\v': c = 'v'; break;
					case '\b': c = 'b'; break;
					case '\f': c = 'f'; break;
					case '\a': c = 'a'; break;
					case '\e': c = 'e'; break;
					case '"': case '\\': break;

					default: {
						if((c >= ' ' && c < 0177) || (b8 && c > 0200))
							out(c);
						else
							out('\\').print(c, "03o");

						continue;
					}
				}
				out('\\')(c);
			}

			out('"');
		}
		break;

		case j: {
			str_t::ptr_t ptr = str;

			out('"');

			while(ptr) {
				unsigned short c = b8 ? *(ptr++) : __ucs2(ptr);

				switch(c) {
					case '\b': c = 'b'; break;
					case '\f': c = 'f'; break;
					case '\n': c = 'n'; break;
					case '\r': c = 'r'; break;
					case '\t': c = 't'; break;
					case '"': case '\\': break;

					default: {
						if((c >= ' ' && c < 0177) || (b8 && c > 0200))
							out(c);
						else
							out('\\')('u').print(c, "04X");

						continue;
					}
				}
				out('\\')(c);
			}

			out('"');
		}
		break;

		default:
			out(str);
	}
}

} // namespace pd
