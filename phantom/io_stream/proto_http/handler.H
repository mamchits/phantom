// This file is part of the phantom::io_stream::proto_http module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/http/server.H>

#include <pd/base/config.H>

#pragma GCC visibility push(default)

namespace phantom {

class scheduler_t;

namespace io_stream { namespace proto_http {

namespace handler {

class verify_t {
public:
	typedef http::remote_request_t request_t;

	virtual void proc(request_t const &request) const = 0;

protected:
	inline verify_t() throw() { }
	inline ~verify_t() throw() { }

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

} // namespace handler

class handler_t {
public:
	typedef http::remote_request_t request_t;
	typedef http::local_reply_t reply_t;

	typedef handler::verify_t verify_t;

private:
	virtual void do_proc(request_t const &request, reply_t &reply) const = 0;

	verify_t const *verify;
	scheduler_t *scheduler;
	interval_t switch_prio;

public:
	struct config_t {
		config_binding_type_ref(verify_t);
		config::objptr_t<verify_t> verify;
		config::objptr_t<scheduler_t> scheduler;
		interval_t switch_prio;

		inline config_t() throw() :
			verify(), scheduler(), switch_prio(interval::zero) { }

		inline void check(in_t::ptr_t const &) const { }

		inline ~config_t() throw() { }
	};

	void proc(request_t const &request, reply_t &reply) const;

protected:
	inline handler_t(string_t const &, config_t const &config) throw() :
		verify(config.verify),
		scheduler(config.scheduler), switch_prio(config.switch_prio) { }

	inline ~handler_t() throw() { }

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

}}} // namespace phantom::io_stream::proto_http

#pragma GCC visibility pop
