// This file is part of the phantom::io_client::proto_none module.
// Copyright (C) 2010-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "instance.I"
#include "entry.I"

#include <pd/bq/bq_job.H>
#include <pd/bq/bq_in.H>
#include <pd/bq/bq_out.H>

#include <pd/base/exception.H>
#include <pd/base/fd_guard.H>

namespace phantom { namespace io_client { namespace proto_none {

bool instance_t::do_send(out_t &out) {
	{
		bq_cond_guard_t guard(out_cond);

		while(true) {
			if(!work)
				return false;

			if(pending)
				break;

			if(!bq_success(out_cond.wait(NULL)))
				throw exception_sys_t(log::error, errno, "out_cond.wait: %m");
		}

		--pending;
	}

	try {
		ref_t<task_t> task = proto.entry->get_task();

		if(task->active()) {
			{
				bq_cond_guard_t guard(in_cond);

				queue.insert(task);

				in_cond.send();
			}

			out.ctl(1);
			task->print(out);
			out.flush_all();
			out.ctl(0);
		}
		else {
			proto.entry->derank_instance(this);
		}
	}
	catch(exception_sys_t const &ex) {
		{
			bq_cond_guard_t guard(in_cond);
			if(work) {
				work = false;
				in_cond.send();
			}
		}

		if(ex.errno_val == ECANCELED)
			throw;

		ex.log();
	}

	return work;
}

bool instance_t::do_recv(in_t::ptr_t &ptr) {
	ref_t<task_t> task = ({
		bq_cond_guard_t guard(in_cond);

		while(true) {
			if(!work)
				return false;

			if(queue.get_count())
				break;

			if(!bq_success(in_cond.wait(NULL)))
				throw exception_sys_t(log::error, errno, "in_cond.wait: %m");
		}

		queue.remove();
	});

	try {
		if(!task->parse(ptr)) { work = false; }

		task->set_ready();
	}
	catch(exception_t const &ex) {
		{
			bq_cond_guard_t guard(out_cond);
			if(work) {
				work = false;
				out_cond.send();
			}
		}

		task->clear();

		ex.log();

		proto.entry->put_task(task);
	}

	proto.entry->derank_instance(this);

	return work;
}

void instance_t::do_cancel() {
	while(true) {
		ref_t<task_t> task = ({
			bq_cond_guard_t guard(in_cond);
			if(!queue.get_count())
				break;

			queue.remove();
		});

		if(task->active())
			proto.entry->put_task(task);
	}
	rank = 0;
	pending = 0;
}

void instance_t::recv_exit_send() {
	bq_cond_guard_t guard(recv_exit_cond);
	recv = false;
	recv_exit_cond.send();
}

void instance_t::recv_exit_wait() {
	bq_cond_guard_t guard(recv_exit_cond);
	while(recv)
		if(!bq_success(recv_exit_cond.wait(NULL))) {
			// We should recevie recv_exit_cond signal at last
			// even if bq_thr doesn't work more

			if(errno == ECANCELED)
				continue;

			throw exception_sys_t(log::error, errno, "recv_exit_cond.wait: %m");
		}
}

void instance_t::recv_proc(bq_conn_t &conn) {
	struct exit_guard_t {
		instance_t &instance;

		inline exit_guard_t(instance_t &_instance) throw() :
			instance(_instance) { }

		inline ~exit_guard_t() throw() { instance.recv_exit_send(); }
	} exit_guard(*this);

	bq_in_t in(conn, proto.prms.ibuf_size, proto.prms.in_timeout);
	in_t::ptr_t ptr(in);

	while(do_recv(ptr)) {
		in.timeout_reset();
		in.truncate(ptr);
	}
}

void instance_t::proc(bq_conn_t &conn) {
	assert(proto.entry);

	work = true;
	recv = true;

	bq_job_t<typeof(&instance_t::recv_proc)>::create(
		log::handler_t::get_label(), bq_thr_get(),
		*this, &instance_t::recv_proc, conn
	);

	{
		struct instance_guard_t {
			entry_t &entry;
			instance_t &instance;

			inline instance_guard_t(entry_t &_entry, instance_t &_instance) throw() :
				entry(_entry), instance(_instance) {

				entry.insert_instance(&instance);
			}

			inline ~instance_guard_t() throw() {
				entry.remove_instance(&instance);
			}

		} instance_guard(*proto.entry, *this);

		char obuf[proto.prms.obuf_size];
		bq_out_t out(conn, obuf, sizeof(obuf), proto.prms.out_timeout);

		while(do_send(out))
			out.timeout_reset();

		recv_exit_wait();
	}

	do_cancel();
}

instance_t::~instance_t() throw() { }

}}} // namespace phantom::io_client::proto_none
