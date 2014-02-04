// This file is part of the phantom::io_client::proto_none module.
// Copyright (C) 2010-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include "task.H"

#include <pd/base/mutex.H>

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

		uint32_t rand_val;
		size_t rand_cnt;
		bool rand();

	public:
		inline instances_t(size_t _max_count) :
			count(0), max_count(_max_count),
			instances(new instance_t *[_max_count]), rand_val(0), rand_cnt(0) { }

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
		mutex_t mutex;
		size_t count, maxcount;
		ref_t<task_t> *tasks;

		typedef stat::count_t tcount_t;
		typedef stat::mmcount_t qcount_t;

		typedef stat::items_t<
			tcount_t,
			qcount_t
		> stat_base_t;

		struct stat_t : stat_base_t {
			inline stat_t() throw() : stat_base_t(
				STRING("tasks"),
				STRING("queue")
			) { }

			inline ~stat_t() throw() { }

			inline tcount_t &tcount() { return item<0>(); }
			inline qcount_t &qcount() { return item<1>(); }
		};

		stat_t stat;

		void place(ref_t<task_t> const &task, size_t i, bool flag);

	public:
		bool insert(ref_t<task_t> const &task);
		ref_t<task_t> remove();

		inline size_t num() const { return count; }

		inline tasks_t() throw() :
			mutex(), count(0), maxcount(0), tasks(0), stat() { }

		inline ~tasks_t() throw() { if(tasks) delete [] tasks; }

		inline void init() { stat::meta(stat, mutex.stat).init(); }
		inline void stat_print() { stat::meta(stat, mutex.stat).print(); }
	};

	tasks_t tasks;

public:
	inline entry_t(size_t _queue_size, size_t _instances_count, size_t _quorum) :
		cond(), pending(0), queue_size(_queue_size), quorum(_quorum),
		instances(_instances_count), tasks() {
	}

	inline ~entry_t() throw() { }

	inline void put_task(ref_t<task_t> &task) {
		task->set_begin();

		bool inc_pending = tasks.insert(task);

		{
			bq_cond_t::handler_t handler(cond);

			if(inc_pending) ++pending;

			handler.send();
		}
	}

	inline ref_t<task_t> get_task() { return tasks.remove(); }

	void insert_instance(instance_t *instance);
	void remove_instance(instance_t *instance);
	void derank_instance(instance_t *instance);

	inline void init() { tasks.init(); }

	void run();

	inline void stat_print() { tasks.stat_print(); }
};

}}} // namespace phantom::io_client::proto_none
