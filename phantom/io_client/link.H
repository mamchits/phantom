// This file is part of the phantom::io_client module.
// Copyright (C) 2010-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/io_client/proto.H>

#include <phantom/scheduler.H>

#include <pd/base/netaddr.H>
#include <pd/base/list.H>
#include <pd/base/size.H>
#include <pd/base/time.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_client {

class link_t {
	interval_t const conn_timeout;
	log::level_t remote_errors;
	proto_t::instance_t *instance;

	virtual netaddr_t const &remote_netaddr() const throw() = 0;
	virtual fd_ctl_t const *ctl() const throw() = 0;

	void loop() const;

public:
	void init();
	void run(scheduler_t const &scheduler) const;
	void stat_print() const;

protected:
	inline link_t(
		unsigned int rank, interval_t _conn_timeout,
		log::level_t _remote_errors, proto_t &_proto
	) :
		conn_timeout(_conn_timeout), remote_errors(_remote_errors),
		instance(_proto.create_instance(rank)) { }

public:
	virtual ~link_t() throw();
};

struct pool_t {
	string_t name;

	struct bucket_t {
		string_t name;

		struct item_t {
			string_t name;

			link_t *link;

			inline item_t() : name(), link(NULL) { }
			inline item_t(item_t const &) = delete;
			inline item_t &operator=(item_t const &) = delete;
			inline ~item_t() throw() { delete link; }

			inline void setup(string_t const &_name, link_t *_link) {
				name = _name;
				link = _link;
			}

			inline void init() {
				log::handler_t handler(name);
				link->init();
			}

			inline void run(scheduler_t const &scheduler) const {
				log::handler_t handler(name);
				link->run(scheduler);
			}

			inline void stat_print() const {
				stat::ctx_t ctx(name.str(), 2);
				link->stat_print();
			}
		};

		size_t size;
		item_t *items;

		inline bucket_t() : name(), size(0), items(NULL) { }
		inline bucket_t(bucket_t const &) = delete;
		inline bucket_t &operator=(bucket_t const &) = delete;
		inline ~bucket_t() { delete [] items; }

		inline void setup(netaddr_t const &netaddr, size_t _size) {
			name = string_t::ctor_t(netaddr.print_len()).print(netaddr);

			delete [] items;
			items = new item_t[size = _size];
		}

		inline void init() {
			log::handler_t handler(name);
			for(size_t i = 0; i < size; ++i) items[i].init();
		}

		inline void run(scheduler_t const &scheduler) const {
			log::handler_t handler(name);
			for(size_t i = 0; i < size; ++i) items[i].run(scheduler);
		}

		inline void stat_print() const {
			stat::ctx_t ctx(name.str(), 2, size);
			for(size_t i = 0; i < size; ++i) items[i].stat_print();
		}

		inline item_t &operator[](size_t ind) { return items[ind]; }
	};

	size_t size;
	bucket_t *buckets;

	inline pool_t() : name(), size(0), buckets(NULL) { }
	inline pool_t(pool_t const &pool_t) = delete;
	inline pool_t &operator=(pool_t const &) = delete;
	inline ~pool_t() throw() { delete [] buckets; }

	inline void setup(string_t const &_name, size_t _size) {
		name = _name;
		delete [] buckets;
		buckets = new bucket_t[size = _size];
	}

	inline void init() {
		log::handler_t handler(name);
		for(size_t i = 0; i < size; ++i) buckets[i].init();
	}

	inline void run(scheduler_t const &scheduler) const {
		log::handler_t handler(name);
		for(size_t i = 0; i < size; ++i) buckets[i].run(scheduler);
	}

	inline void stat_print() const {
		stat::ctx_t ctx(name.str(), 2, size);
		for(size_t i = 0; i < size; ++i) buckets[i].stat_print();
	}

	inline bucket_t &operator[](size_t ind) throw() { return buckets[ind]; }
};

class links_t {
protected:
	string_t name;
	size_t count;
	unsigned int rank;

public:
	struct config_t {
		sizeval_t count;
		sizeval_t rank;

		inline config_t() : count(1), rank(0) { }
		inline void check(in_t::ptr_t const &ptr) const {
			if(count > sizeval::kilo)
				config::error(ptr, "count is too big");

			if(rank > sizeval::kilo)
				config::error(ptr, "rank is too big");
		}
		inline ~config_t() throw() { }
	};

	virtual void create(
		pool_t &pool, interval_t _conn_timeout, log::level_t _remote_errors, proto_t &_proto
	) const = 0;

protected:
	inline links_t(string_t const &_name, config_t const &config) :
		name(_name), count(config.count), rank(config.rank) { }

	inline ~links_t() throw() { }

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

}} // namespace phantom::io_client

#pragma GCC visibility pop
