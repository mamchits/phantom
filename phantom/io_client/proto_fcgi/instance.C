// This file is part of the phantom::io_client::proto_fcgi module.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "instance.I"
#include "entry.I"
#include "fcgi.I"

#include <pd/bq/bq_job.H>
#include <pd/bq/bq_in.H>

#include <pd/base/exception.H>
#include <pd/base/fd_guard.H>

namespace phantom { namespace io_client { namespace proto_fcgi {

bool instance_t::do_recv(in_t::ptr_t &ptr) {
	{
		bq_cond_t::handler_t handler(in_cond);
		while(true) {
			if(!work)
				return false;

			if(tasks.count())
				break;

			handler.wait();
		}
	}

	if(!ptr) {
		log_warning("connection closed by application server");
		return false;
	}

	record_t record = parse_record(ptr);

	if(record.id) {
		ref_t<task_t> task = tasks[record.id];
		if(!task) {
			log_warning("fcgi response of type %u for deleted task", record.type);
			return true;
		}

		switch(record.type) {
			case type_stdout:
				task->stdout.append(record.data);
			break;

			case type_stderr:
				task->stderr.append(record.data);
			break;

			case type_end_request:
				tasks.remove(task);
				{
					uint32_t app_status;
					uint8_t code;

					decode_end_request(record.data, app_status, code);

					if(!code)
						task->set_ready(app_status);
					else
						task->cancel(code);
				}
			break;

			default:
				log_warning("fcgi response of type %u, id %u", record.type, record.id);
		}
	}
	else {
		if(record.type == type_get_values_result)
			log_get_values_result(record.data);
		else
			log_warning("fcgi response of type %u, id 0", record.type);
	}

	return true;
}

static string_t const val_max_conns = STRING("FCGI_MAX_CONNS");
static string_t const val_max_reqs = STRING("FCGI_MAX_REQS");
static string_t const val_mpxs_conns = STRING("FCGI_MPXS_CONNS");

void instance_t::proc(bq_conn_t &conn) {
	assert(proto.entry);

	char obuf[proto.prms.obuf_size];
	bq_out_t out(conn, obuf, sizeof(obuf));

	bq_in_t in(conn, proto.prms.ibuf_size);
	in_t::ptr_t ptr(in);

#if 0
	{
		params_t params(3);
		params.add(val_max_conns, string_t::empty);
		params.add(val_max_reqs, string_t::empty);
		params.add(val_mpxs_conns, string_t::empty);

		out.timeout_set(proto.prms.out_timeout);

		out.ctl(1);
		record_t(type_get_values, 0, params).print(out);
		out.flush_all();
		out.ctl(0);
	}
#endif
#if 0
	{
		in.timeout_set(proto.prms.in_timeout);

		record_t record = parse_record(ptr);

		if(record.id || record.type != type_get_values_result) {
			log_error("insane application server");
			return;
		}

		log_get_values_result(record.data);

		in.truncate(ptr);
	}
#endif

	pout = &out;
	work = true;

	{
		proto.entry->insert_instance(this);

		out_guard.relax();

		try {
			while(true) {
				in.timeout_set(proto.prms.in_timeout);
				if(!do_recv(ptr))
					break;
				in.truncate(ptr);
			}
		}
		catch(...) { }

		while(true) {
			try {
				out_guard.wakeup();
				break;
			}
			catch(...) { }
		}

		proto.entry->remove_instance(this);
	}

	tasks.cancel();

	pout = NULL;
}

static string_t const request_method_key = STRING("REQUEST_METHOD");
static string_t const server_name_key = STRING("SERVER_NAME");
static string_t const server_port_key = STRING("SERVER_PORT");
static string_t const server_protocol_key = STRING("SERVER_PROTOCOL");
static string_t const script_name_key = STRING("SCRIPT_NAME");
static string_t const root_name_key = STRING("DOCUMENT_ROOT");
static string_t const script_filename_key = STRING("SCRIPT_FILENAME");
static string_t const query_string_key = STRING("QUERY_STRING");

static string_t const gateway_interface_key = STRING("GATEWAY_INTERFACE");
static string_t const gateway_interface_val = STRING("CGI/1.1");

static string_t const request_method_post_val = STRING("POST");
static string_t const request_method_get_val = STRING("GET");

static string_t const server_port_val = STRING("80");
static string_t const server_protocol_val =  STRING("HTTP/1.1");

ref_t<task_t> instance_t::create_task(
	request_t const &request, interval_t *timeout, string_t const &root
) {
	bq_err_t res = out_mutex.lock(timeout);

	if(res != bq_ok) {
		if(res == bq_timeout)
			return ref_t<task_t>();

		bq_success(res);
		throw exception_sys_t(log::error, errno, "out_mutex.lock: %m");
	}

	ref_t<task_t> task = tasks.create();
	if(task) {
		assert(pout != NULL);
		bq_out_t &out = *pout;
		out.timeout_set(*timeout);

		{
			// TODO: make in_cond protect tasks

			bq_cond_t::handler_t handler(in_cond);
			handler.send();
		}

		try {
			uint16_t id = task->id;

			out.ctl(1);

			record_t(type_begin_request, id, begin_request_body).print(out);
			{
				params_t params(6);

				params.add(
					request_method_key,
					request.method == http::method_post
						? request_method_post_val
						: request_method_get_val
				);

				params.add(server_name_key, request.host);
				params.add(server_port_key, server_port_val); // FIXME
				params.add(server_protocol_key, server_protocol_val);
				params.add(script_name_key, request.path);

				if(root) {
					params.add(root_name_key, root);

					string_t const script_filename_val = ({
						string_t::ctor_t ctor(root.size() + 1 + request.path.size());
						ctor(root)('/')(request.path);
						(string_t)ctor;
					});

					params.add(script_filename_key, script_filename_val);
				}

				params.add(query_string_key, request.uri_args);

				record_t(type_params, id, params).print(out);
			}

			record_t(type_stdin, id, request.entity).print(out);

			out.flush_all();
			out.ctl(0);
		}
		catch(...) {
			bq_cond_t::handler_t handler(in_cond);
			work = false;
			handler.send();

			task = ref_t<task_t>();
		}

		*timeout = out.timeout_get();
	}

	out_mutex.unlock();

	return task;
}

void instance_t::do_abort(ref_t<task_t> task) {
	interval_t timeout = proto.prms.out_timeout;

	int res = out_mutex.lock(&timeout);

	if(res != bq_ok) {
		if(task->status == task_t::processed)
			tasks.remove(task);

		return;
	}

	if(task->status == task_t::processed) {
		assert(pout != NULL);
		bq_out_t &out = *pout;
		out.timeout_set(timeout);

		try {
			uint16_t id = task->id;

			out.ctl(1);
			record_t(type_abort_request, id).print(out);
			out.flush_all();
			out.ctl(0);
		}
		catch(...) {
			bq_cond_t::handler_t handler(in_cond);
			work = false;
			handler.send();
		}
	}

	out_mutex.unlock();

	return;
}

static string_t const abort_label = STRING("abort");

void instance_t::abort(task_t *task) {
	log::handler_t handler(abort_label);
	bq_job(&instance_t::do_abort)(*this, task)->run(bq_thr_get());
}

void instance_t::init() { }

void instance_t::stat_print() { }

instance_t::~instance_t() throw() {
	assert(trank == 0);
	assert(pout == 0);
}

}}} // namespace phantom::io_client::proto_fcgi
