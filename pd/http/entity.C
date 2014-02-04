// This file is part of the pd::http library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "http.H"

namespace pd { namespace http {

static inline bool hexdigit(char c, size_t &res) {
	switch(c) {
		case '0'...'9': res = c - '0'; return true;
		case 'a'...'f': res = 10 + (c - 'a'); return true;
		case 'A'...'F': res = 10 + (c - 'A'); return true;
	}
	return false;
}

bool number_parse(in_segment_t const &str, size_t &num) {
	in_t::ptr_t ptr = str;
	size_t res;

	while(true) {
		if(!ptr) return false;
		char c = *ptr;
		if(c != ' ' && c != '\t' && c != '\r' && c != '\n') break;
		++ptr;
	}

	if(!ptr.parse(res))
		return false;

	while(ptr) {
		char c = *ptr;
		if(c != ' ' && c != '\t' && c != '\r' && c != '\n') return false;
		++ptr;
	}

	num = res;
	return true;
}

bool entity_t::parse(
	in_t::ptr_t &ptr, eol_t const &eol, mime_header_t const &header,
	limits_t const &limits, bool reply
) {
	if(size())
		clear();

	in_segment_t const *val = header.lookup(CSTR("transfer-encoding"));
	if(val) {
		if(!token_find(*val, CSTR("chunked")))
			throw exception_t(code_400, "Unknown transfer encoding");

		while(true) {
			size_t len = 0, d = 0;

			while(true) {
				char c = *ptr;

				if(c == ';') {
					size_t limit = 1024;
					if(!ptr.scan("\r\n", 2, limit))
						throw exception_t(code_400, "Junk after chunk size");

					break;
				}

				if(c == '\n' || c == '\r') break;

				if(!hexdigit(c, d))
					throw exception_t(code_400, "Wrong digit in chunk size");

				len = ({
					size_t tlen = len * 16 + d;

					if(tlen / 16 != len)
						throw exception_t(code_400, "Chunk too big");

					tlen;
				});

				++ptr;
			}

			if(!eol.check(ptr))
				throw exception_t(code_400, "Wrong EOL after chunk size");

			if(!len) break;

			if(size() + len > limits.entity_size)
				throw exception_t(code_413, "Entity too big");

			append(in_segment_t(ptr, len));
			ptr += len;

			if(!eol.check(ptr))
				throw exception_t(code_400, "Wrong EOL after chunk");
		}

		trailer.parse(ptr, eol, limits);

		return true;
	}

	val = header.lookup(CSTR("content-length"));
	if(val) {
		size_t len = 0;

		if(!number_parse(*val, len))
			throw exception_t(code_400, "Not a number in Content-Length");

		if(len) {
			if(len > limits.entity_size)
				throw exception_t(code_413, "Entity too big");

			append(in_segment_t(ptr, len));
			ptr += len;
		}

		return true;
	}

	if(reply) {
		in_t::ptr_t p0 = ptr;

		while(ptr)
			ptr.seek_end();

		append(in_segment_t(p0, ptr - p0));
	}

	return false;
}

}} // namespace pd::http
