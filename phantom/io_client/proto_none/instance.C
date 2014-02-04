// This file is part of the phantom::io_client::proto_none module.
// Copyright (C) 2010-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2014, YANDEX LLC.
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

void instance_t::do_send(out_t &out) {
	stat.send_tstate().set(send::idle);

	{
		bq_cond_t::handler_t handler(out_cond);

		while(true) {
			if(!work)
				return;

			if(pending)
				break;

			handler.wait();
		}

		--pending;
	}

	stat.send_tstate().set(send::run);

	ref_t<task_t> task = proto.entry->get_task();

	if(task->active()) {
		{
			bq_cond_t::handler_t handler(in_cond);

			queue.insert(task);

			handler.send();
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

void instance_t::do_recv(in_t::ptr_t &ptr) {
	stat.recv_tstate().set(recv::idle);

	ref_t<task_t> task = ({
		bq_cond_t::handler_t handler(in_cond);

		while(true) {
			if(!work)
				return;

			if(queue.get_count())
				break;

			handler.wait();
		}

		queue.remove();
	});

	stat.recv_tstate().set(recv::run);

	try {
		if(!task->parse(ptr)) { work = false; }

		++stat.tcount();

		task->set_ready();
	}
	catch(...) {
		task->clear();
		proto.entry->put_task(task);

		throw;
	}

	proto.entry->derank_instance(this);
}

template<typename c_t>
struct cleanup_t {
	c_t const &c;
	inline cleanup_t(c_t const &_c) : c(_c) { }
	inline ~cleanup_t() { c(); }
};

void instance_t::recv_proc(bq_conn_t &conn) {
	auto cleanup_f = [this]() -> void {
		bq_cond_t::handler_t handler(out_cond);
		work = false;
		recv = false;
		handler.send();
	};

	cleanup_t<typeof(cleanup_f)> cleanup(cleanup_f);

	bq_in_t in(conn, proto.prms.ibuf_size, &stat.icount());
	in_t::ptr_t ptr(in);

	while(work) {
		in.timeout_set(proto.prms.in_timeout);
		do_recv(ptr);
		in.truncate(ptr);
	}
}

static string_t const recv_label = STRING("recv");
static string_t const send_label = STRING("send");

void instance_t::proc(bq_conn_t &conn) {
	assert(proto.entry);

	++stat.conns();

	work = true;
	recv = true;

	{
		log::handler_t handler(recv_label);
		bq_job(&instance_t::recv_proc)(*this, conn)->run(bq_thr_get());
	}

	proto.entry->insert_instance(this);

	auto cleanup_f = [this]() -> void {
		{
			bq_cond_t::handler_t handler(in_cond);
			work = false;
			handler.send();
		}

		{
			bq_cond_t::handler_t handler(out_cond);

			// We should wait the end of the recv_proc here
			// even if bq_thr doesn't work more

			// FIXME: deadlock on shutdown rare possible here

			while(recv) {
				try { handler.wait(); } catch(...) { }
			}
		}

		proto.entry->remove_instance(this);

		while(true) {
			ref_t<task_t> task = ({
				bq_cond_t::guard_t guard(in_cond);
				if(!queue.get_count())
					break;

				queue.remove();
			});

			if(task->active())
				proto.entry->put_task(task);
		}
		trank = 0;
		pending = 0;

		stat.send_tstate().set(send::connect);
	};

	cleanup_t<typeof(cleanup_f)> cleanup(cleanup_f);

	log::handler_t handler(send_label);

	char obuf[proto.prms.obuf_size];
	bq_out_t out(conn, obuf, sizeof(obuf), &stat.ocount());

	while(work) {
		out.timeout_set(proto.prms.out_timeout);
		do_send(out);
	}
}

void instance_t::init() { stat.init(); }

void instance_t::stat_print() { stat.print(); }

instance_t::~instance_t() throw() { }

}}} // namespace phantom::io_client::proto_none
