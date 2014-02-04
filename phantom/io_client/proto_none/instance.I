// This file is part of the phantom::io_client::proto_none module.
// Copyright (C) 2010-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include "task.H"
#include "proto_none.H"

#include <pd/base/queue.H>
#include <pd/base/stat.H>
#include <pd/base/stat_items.H>

namespace phantom { namespace io_client { namespace proto_none {

namespace send {
enum state_t { connect, idle, run };
}

namespace recv {
enum state_t { idle, run };
}

class instance_t : public proto_t::instance_t {
	proto_none_t const &proto;

	typedef stat::count_t conns_t;
	typedef stat::count_t tcount_t;
	typedef stat::count_t icount_t;
	typedef stat::count_t ocount_t;
	typedef stat::mmcount_t qcount_t;

	struct send_tstate_t : stat::tstate_t<send::state_t, 3> {
		inline send_tstate_t() :
			stat::tstate_t<send::state_t, 3>(
				STRING("connect"), STRING("idle"), STRING("run")
			) { }
	};

	struct recv_tstate_t : stat::tstate_t<recv::state_t, 2> {
		inline recv_tstate_t() :
			stat::tstate_t<recv::state_t, 2>(
				STRING("idle"), STRING("run")
			) { }
	};

	typedef stat::items_t<
		conns_t, tcount_t, icount_t, ocount_t, qcount_t,
		send_tstate_t, recv_tstate_t
	> stat_base_t;

	struct stat_t : stat_base_t {
		inline stat_t() throw() : stat_base_t(
			STRING("conns"),
			STRING("tasks"),
			STRING("in"),
			STRING("out"),
			STRING("queue"),
			STRING("send_tstate"),
			STRING("recv_tstate")
		) { }

		inline ~stat_t() throw() { }

		inline conns_t &conns() { return item<0>(); }
		inline tcount_t &tcount() { return item<1>(); }
		inline icount_t &icount() { return item<2>(); }
		inline ocount_t &ocount() { return item<3>(); }
		inline qcount_t &qcount() { return item<4>(); }
		inline send_tstate_t &send_tstate() { return item<5>(); }
		inline recv_tstate_t &recv_tstate() { return item<6>(); }
	};

	stat_t stat;

	bool work;

	bq_cond_t in_cond;
	queue_t<ref_t<task_t>> queue;

	bq_cond_t out_cond;

	bool recv;

	void do_send(out_t &out);
	void do_recv(in_t::ptr_t &ptr);

	void recv_proc(bq_conn_t &conn);

public:
	unsigned int pending;
	unsigned int trank;
	size_t ind;

	inline unsigned int rank() { return trank + drank; }

	inline void send_task() {
		bq_cond_t::handler_t handler(out_cond);
		++pending;
		handler.send();
	}

	inline instance_t(proto_none_t const &_proto, unsigned int _rank) :
		proto_t::instance_t(_rank), proto(_proto), stat(),
		work(false), in_cond(), queue(proto.prms.queue_size, stat.qcount()),
		out_cond(), recv(false), pending(0), trank(0), ind(0) { }

private:
	virtual void init();
	virtual void proc(bq_conn_t &conn);
	virtual void stat_print();
	virtual ~instance_t() throw();
};

}}} // namespace phantom::io_client::proto_none
