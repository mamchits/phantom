// This file is part of the phantom::io_client::proto_fcgi module.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "fcgi.I"

#include <pd/http/http.H>

#include <pd/base/exception.H>

namespace phantom { namespace io_client { namespace proto_fcgi {

static inline uint16_t parse_uint16(in_t::ptr_t &ptr) {
	unsigned char c1 = *(ptr++);
	unsigned char c0 = *(ptr++);

	return (((uint16_t)c1) << 8) | c0;
}

static inline uint32_t parse_uint32(in_t::ptr_t &ptr) {
	unsigned char c3 = *(ptr++);
	unsigned char c2 = *(ptr++);
	unsigned char c1 = *(ptr++);
	unsigned char c0 = *(ptr++);

	return
		(((uint32_t)c3) << 24) | (((uint32_t)c2) << 16) |
		(((uint32_t)c1) << 8) | c0
	;
}

static inline uint32_t parse_len(in_t::ptr_t &ptr) {
	unsigned int c3 = *(ptr++);
	if(c3 < 0x80)
		return c3;

	unsigned char c2 = *(ptr++);
	unsigned char c1 = *(ptr++);
	unsigned char c0 = *(ptr++);

	return
		(((uint32_t)(c3 & 0x7f)) << 24) | (((uint32_t)c2) << 16) |
		(((uint32_t)c1) << 8) | c0
	;
}

record_t parse_record(in_t::ptr_t &ptr) {
	record_t res;
	in_t::ptr_t p = ptr;

	if(!p.match<ident_t>(1)) {
		log_error("Illegal fcgi record version");
		throw http::exception_t(http::code_502, "Bad Gateway");
	}

	res.type = (type_t)*(p++);
	res.id = parse_uint16(p);

	uint16_t size = parse_uint16(p);
	uint8_t pad_size = *(p++);
	++p; // skip reserved;

	res.data = in_segment_t(p, size);
	p += (size + pad_size);

	ptr = p;

	log_debug("fcgi recv id: %u, type: %u, len %u", res.id, res.type, size);
	return res;
}

void decode_end_request(
	in_segment_t const &status, uint32_t &app_status, uint8_t &code
) {
	in_t::ptr_t ptr = status;
	app_status = parse_uint32(ptr);
	code = *ptr;
}

void log_get_values_result(in_segment_t const &data) {
	in_t::ptr_t ptr = data;
	while(ptr) {
		size_t key_len = parse_len(ptr);
		size_t val_len = parse_len(ptr);

		string_t key(ptr, key_len);
		ptr += key_len;
		string_t val(ptr, val_len);
		ptr += val_len;

		log_info(
			"\"%.*s\" -> \"%.*s\"",
			(int)key.size(), key.ptr(), (int)val.size(), val.ptr()
		);
	}
}

static inline void print_uint16(out_t &out, uint16_t val) {
	out((unsigned char)(val >> 8))((unsigned char)(val));
}

static inline void print_uint32(out_t &out, uint32_t val) {
	out
		((unsigned char)(val >> 24))((unsigned char)(val >> 16))
		((unsigned char)(val >> 8))((unsigned char)(val))
	;
}

static void do_print(
	out_t &out, type_t type, uint16_t id, in_segment_t const &data
) {
	out(1); // version
	out(type);
	print_uint16(out, id);

	size_t len = data.size();
	assert(len <= 0xfff8);

	uint8_t d = len % 8;
	uint8_t pad = d ? 8 - d : 0;

	print_uint16(out, (uint16_t) len);
	out(pad);
	out(0); // reserved
	out(data);
	out(str_t("\0\0\0\0\0\0\0\0", pad));

	log_debug("fcgi send id: %u, type: %u, len %zu", id, type, len);
}

string_t const begin_request_body = STRING(
	"\0\1" "\1" "\0\0\0\0\0" // role = 1, flags = keep_conn, reserved = { 0, }
);

void record_t::print(out_t &out) {
	switch(type) {
		case type_params:
		case type_stdin:
		case type_stdout:
		case type_stderr:
		case type_data:
		{
			size_t size = data.size();
			in_t::ptr_t ptr = data;

			while(size) {
				size_t psize = min(size, (size_t)0xfff8);
				do_print(out, type, id, in_segment_t(ptr, psize));
				ptr += psize;
				size -= psize;
			}

			do_print(out, type, id, string_t::empty);
		}
		break;

		default:
			do_print(out, type, id, data);
	}
}

// FIXME: two allocs per param and full copy are not good.

void params_t::add(in_segment_t const &key, in_segment_t const &val) {
	size_t key_size = key.size();
	size_t val_size = val.size();
	string_t::ctor_t ctor(8 + key_size + val_size);

	if(key_size < 128)
		ctor(key_size);
	else
		print_uint32(ctor, key_size | 0x80000000);

	if(val_size < 128)
		ctor(val_size);
	else
		print_uint32(ctor, val_size | 0x80000000);

	ctor(key);

	ctor(val);

	append(string_t(ctor));
}

}}} // namespace phantom::io_client::proto_fcgi
