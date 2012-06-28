// This file is part of the phantom::io_client::proto_none module.
// Copyright (C) 2010-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include "task.H"

namespace phantom { namespace io_client { namespace proto_none {

class instance_t;

class entry_t {
	bq_cond_t cond;
	unsigned int pending;

	size_t const queue_size;
	size_t const quorum;

	class instances_t {
		size_t count;
		size_t const max_count;

		instance_t **instances;

		void put(instance_t *instance, size_t ind);
		instance_t *get(size_t ind) const;
		void place(instance_t *instance, size_t ind, bool flag);

		static bool hcmp(instance_t *instance1, instance_t *instance2);

	public:
		inline instances_t(size_t _max_count) :
			count(0), max_count(_max_count),
			instances(new instance_t *[_max_count]) { }

		inline ~instances_t() throw() { delete [] instances; }

		void insert(instance_t *instance);
		void remove(instance_t *instance);
		void dec_rank(instance_t *instance);
		void inc_rank(instance_t *instance);

		inline instance_t *head() const { return get(1); }
		inline size_t get_count() const { return count; }
	};

	instances_t instances;

	class tasks_t {
		size_t count, maxcount;
		ref_t<task_t> *tasks;

		void place(ref_t<task_t> const &task, size_t i, bool flag);
	public:

		bool insert(ref_t<task_t> const &task);
		ref_t<task_t> remove();

		inline size_t num() const { return count; }
		inline tasks_t() throw() : count(0), maxcount(0), tasks(0) { }
		inline ~tasks_t() throw() { if(tasks) delete [] tasks; }
	};

	thr::spinlock_t task_spinlock;
	tasks_t tasks;

public:
	inline entry_t(size_t _queue_size, size_t _instances_count, size_t _quorum) :
		cond(), pending(0), queue_size(_queue_size), quorum(_quorum),
		instances(_instances_count), task_spinlock(), tasks() {
	}

	inline ~entry_t() throw() { }

	inline void put_task(ref_t<task_t> &task) {
		task->set_begin();

		bool inc_pending = ({
			thr::spinlock_guard_t guard(task_spinlock);
			tasks.insert(task);
		});

		{
			bq_cond_guard_t guard(cond);

			if(inc_pending) ++pending;

			cond.send();
		}
	}

	inline ref_t<task_t> get_task() {
		thr::spinlock_guard_t guard(task_spinlock);
		return tasks.remove();
	}

	void insert_instance(instance_t *instance);
	void remove_instance(instance_t *instance);
	void derank_instance(instance_t *instance);

	void run();
	void stat(out_t &out, bool clear);
};

}}} // namespace phantom::io_client::proto_none
