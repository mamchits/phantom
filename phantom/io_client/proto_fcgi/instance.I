// This file is part of the phantom::io_client::proto_fcgi module.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include "task.I"
#include "proto_fcgi.H"

namespace phantom { namespace io_client { namespace proto_fcgi {

class instance_t : public proto_t::instance_t {
	typedef http::remote_request_t request_t;

	proto_fcgi_t const &proto;

	bq_mutex_t out_mutex;
	bq_out_t *pout;
	bq_mutex_guard_t out_guard;

	class tasks_t {
		spinlock_t spinlock;
		uint16_t const size;
		ref_t<task_t> *tasks;
		size_t tasks_count;

	public:
		inline tasks_t(uint16_t _size) :
			spinlock(), size(_size),
			tasks(new ref_t<task_t>[size]), tasks_count(0) { }

		inline ~tasks_t() throw() {
			assert(tasks_count == 0);
			delete [] tasks;
		}

		inline ref_t<task_t> create() {
			task_t *task = new task_t();
			{
				spinlock_guard_t guard(spinlock);
				for(unsigned int _id = 1; _id < size; ++_id) {
					if(!tasks[_id]) {
						task->id = _id;
						tasks[_id] = task;
						++tasks_count;
						return tasks[_id];
					}
				}
				delete task;

				return ref_t<task_t>();
			}
		}

		inline ref_t<task_t> operator[](uint16_t _id) {
			if(_id >= size)
				return ref_t<task_t>();

			{
				spinlock_guard_t guard(spinlock);
				return tasks[_id];
			}
		}

		inline void remove(task_t *task) {
			uint16_t _id = task->id;
			if(_id < size) {
				spinlock_guard_t guard(spinlock);
				if(tasks[_id]) {
					--tasks_count;
					tasks[_id] = NULL;
				}
			}
		}

		inline void cancel() {
			spinlock_guard_t guard(spinlock);
			for(unsigned int _id = 0; _id < size; ++_id) {
				if(tasks[_id]) {
					tasks[_id]->cancel(0);
					--tasks_count;
					tasks[_id] = NULL;
				}
			}
		}

		size_t count() const { return tasks_count; }
	};

	tasks_t tasks;
	bq_cond_t in_cond;

	bool work;

	bool do_recv(in_t::ptr_t &ptr);
	void do_abort(ref_t<task_t> task);

public:
	unsigned int trank;
	size_t ind;

	inline unsigned int rank() { return trank + drank; }

	ref_t<task_t> create_task(
		request_t const &request, interval_t *timeout, string_t const &root
	);
	void abort(task_t *task);

	inline instance_t(proto_fcgi_t const &_proto, unsigned int _rank) :
		proto_t::instance_t(_rank),
		proto(_proto),
		out_mutex(), pout(NULL), out_guard(out_mutex),
		tasks(proto.prms.queue_size),
		in_cond(), work(false), trank(0), ind(0) { }

private:
	virtual void init();
	virtual void proc(bq_conn_t &conn);
	virtual void stat_print();
	virtual ~instance_t() throw();
};

}}} // namespace phantom::io_client::proto_fcgi
