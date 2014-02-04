// This file is part of the phantom::io_client::proto_fcgi module.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include "../../pd.H"

#include <pd/bq/bq_cond.H>
#include <pd/bq/bq_mutex.H>
#include <pd/bq/bq_out.H>

namespace phantom { namespace io_client { namespace proto_fcgi {

struct task_t : public ref_count_atomic_t {
	uint16_t id;
	bq_cond_t cond;
	enum { processed, ready, canceled } status;

	in_segment_list_t stdout, stderr;
	uint32_t app_status;
	uint8_t proto_code;

	inline task_t() :
		id(0), cond(), status(processed),
		stdout(), stderr(), app_status(0), proto_code(0) { }

	inline void set_ready(uint32_t _app_status) {
		bq_cond_t::handler_t handler(cond);
		status = ready;
		app_status = _app_status;
		handler.send();
	}

	inline void cancel(uint8_t _code) {
		bq_cond_t::handler_t handler(cond);
		status = canceled;
		proto_code = _code;
		handler.send();
	}

	inline bool wait(interval_t *timeout) {
		bq_cond_t::handler_t handler(cond);

		while(true) {
			if(status != processed)
				return status == ready;

			if(handler.wait(timeout) != bq_ok)
				return false;
		}
	}

	friend class ref_t<task_t>;
};

}}} // namespace phantom::io_client::proto_fcgi
