// This file is part of the pd::http library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "http.H"

namespace pd { namespace http {

// RFC 2616, 2.2:
//
// CTL            = <any US-ASCII control character
//                  (octets 0 - 31) and DEL (127)>
// token          = 1*<any CHAR except CTLs or separators>
// separators     = "(" | ")" | "<" | ">" | "@"
//                | "," | ";" | ":" | "\" | <">
//                | "/" | "[" | "]" | "?" | "="
//                | "{" | "}" | SP | HT

static inline bool token_char(char c) {
	switch((unsigned char)c) {
		case 0 ... 31: case 127:
		case '(': case ')': case '<': case '>': case '@': case ',':
		case ';': case ':': case '\\': case '"': case '/': case '[':
		case ']': case '?': case '=': case '{': case '}': case ' ':
		case 128 ... 255:
			return false;
	}
	return true;
}

bool token_find(in_segment_t const &str, str_t token) {
	in_t::ptr_t ptr = str;

	while(ptr) {
		if(token_char(*ptr)) {
			if(ptr.match<lower_t>(token) && (!ptr || !token_char(*ptr)))
				return true;

			while(token_char(*ptr))
				if(!++ptr) return false;

		}
		++ptr;
	}
	return false;
}

void mime_header_t::parse_item(
	in_t::ptr_t &p, eol_t const &eol, limits_t const &limits, size_t depth
) {
	if(depth >= limits.field_num)
		throw exception_t(code_400, "too many request-header fields");

	if(*p == '\r' || *p =='\n') {
		if(!eol.check(p))
			throw exception_t(code_400, "wrong EOL");

		if(depth)
			items = new item_t[count = depth];

		return;
	}

	in_t::ptr_t p0 = p;
	size_t limit = limits.field_size;

	while(token_char(*p)) {
		++p;
		if(!limit--)
			throw exception_t(code_400, "request-header key too large");
	}

	if(p == p0 || *p != ':')
		throw exception_t(code_400, "illegal character in key");

	in_segment_t key(p0, (p - p0));

	p0 = ++p;

	do {
		if(!p.scan("\r\n", 2, limit))
			throw exception_t(code_400, "request-header value too large");

		if(!eol.check(p))
			throw exception_t(code_400, "wrong EOL");

	} while(*p == ' ' || *p == '\t');

	in_segment_t val(p0, p - p0);

	parse_item(p, eol, limits, depth + 1);

	item_t &item = items[depth];

	item.key = key;
	item.val = val;

	size_t ind = index(key);
	item.next = items[ind].first;
	items[ind].first = &item;

	return;
}

mime_header_t::item_t const *mime_header_t::__lookup(str_t key) const {
	if(!count) return NULL;

	item_t const *item = items[index(key)].first;

	while(item) {
		if(item->key.cmp_eq<lower_t>(key))
			break;

		item = item->next;
	}

	return item;
}

version_t parse_version(in_t::ptr_t &p) {
	unsigned short major, minor;

	if(!p.match<lower_t>(CSTR("HTTP/")))
		return version_undefined;

	if(!p.parse(major) || !p.match<ident_t>('.') || !p.parse(minor))
		return version_undefined;

	if(major == 1)
		return (minor == 0) ? version_1_0 : version_1_1;
	else if(major > 1)
		return version_1_1;
	else
		return version_1_0;
}

}} // namespace pd::http
