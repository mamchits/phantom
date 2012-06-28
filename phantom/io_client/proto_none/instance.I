// This file is part of the phantom::io_client::proto_none module.
// Copyright (C) 2010-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include "task.H"
#include "proto_none.H"

#include <pd/base/queue.H>

namespace phantom { namespace io_client { namespace proto_none {

class instance_t : public proto_instance_t {
	proto_none_t const &proto;

	bool work;

	bq_cond_t in_cond;
	queue_t<ref_t<task_t> > queue;

	bq_cond_t out_cond;

	bool recv;
	bq_cond_t recv_exit_cond;

	bool do_send(out_t &out);
	bool do_recv(in_t::ptr_t &ptr);
	void do_cancel();

	void recv_proc(bq_conn_t &conn);

	void recv_exit_send();
	void recv_exit_wait();

public:
	unsigned int pending;
	unsigned int rank;
	size_t ind;

	inline void send_task() {
		bq_cond_guard_t guard(out_cond);
		++pending;
		out_cond.send();
	}

	inline instance_t(proto_none_t const &_proto) :
		proto_instance_t(), proto(_proto),
		work(false), in_cond(), queue(proto.prms.queue_size), out_cond(),
		recv(false), recv_exit_cond(), pending(0), rank(0), ind(0) { }

private:
	virtual void proc(bq_conn_t &conn);
	virtual ~instance_t() throw();
};

}}} // namespace phantom::io_client::proto_none
