// This file is part of the phantom::io_client::proto_fcgi module.
// Copyright (C) 2011, 2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011, 2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include "../../pd.H"

#include <pd/http/server.H>
#include <pd/base/thr.H>

namespace phantom { namespace io_client { namespace proto_fcgi {

class instance_t;

class entry_t {
	typedef http::remote_request_t request_t;
	typedef http::local_reply_t reply_t;

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

	thr::spinlock_t instances_spinlock;
	instances_t instances;

	class content_t;
public:
	inline entry_t(size_t _queue_size, size_t _instances_count, size_t _quorum) :
		queue_size(_queue_size), quorum(_quorum),
		instances_spinlock(), instances(_instances_count) { }

	inline ~entry_t() throw() { }

	inline void insert_instance(instance_t *instance) {
		thr::spinlock_guard_t guard(instances_spinlock);
		instances.insert(instance);
	}

	inline void remove_instance(instance_t *instance) {
		thr::spinlock_guard_t guard(instances_spinlock);
		instances.remove(instance);
	}

	bool proc(
		request_t const &request, reply_t &reply,
		interval_t *timeout, string_t const &root
	);

	void stat(out_t &out, bool clear);
};

}}} // namespace phantom::io_client::proto_fcgi
