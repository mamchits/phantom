// This file is part of the phantom::io_client::proto_fcgi module.
// Copyright (C) 2011, 2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011, 2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include "../../pd.H"

#include <pd/base/string.H>

namespace phantom { namespace io_client { namespace proto_fcgi {

enum type_t {
	type_unset = 0,
	type_begin_request = 1,
	type_abort_request = 2,
	type_end_request = 3,
	type_params = 4,
	type_stdin = 5,
	type_stdout = 6,
	type_stderr = 7,
	type_data = 8,
	type_get_values = 9,
	type_get_values_result = 10,
	type_unknown = 11,
};

struct record_t {
	type_t type;
	uint16_t id;
	in_segment_t data;

	inline record_t() : type(type_unset), id(0), data() { }

	inline record_t(
		type_t _type, uint16_t _id, in_segment_t const &_data = string_t::empty
	) : type(_type), id(_id), data(_data) { }

	void print(out_t &out);
};

extern string_t const begin_request_body;

record_t parse_record(in_t::ptr_t &ptr);

enum code_t {
	code_request_complete = 0,
	code_cant_mpx_conn = 1,
	code_overloaded = 2,
	code_unknown_role = 3,
};

void decode_end_request(
	in_segment_t const &status, uint32_t &app_status, uint8_t &proto_code
);

void log_get_values_result(in_segment_t const &data);

class params_t : public in_segment_list_t {
public:
	inline params_t(int) : in_segment_list_t() { }

	void add(in_segment_t const &key, in_segment_t const &val);
};

}}} // namespace phantom::io_client::proto_fcgi
