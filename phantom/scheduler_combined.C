// This file is part of the phantom program.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "scheduler.H"

#include <pd/bq/bq_heap.H>

#include <pd/base/thr.H>
#include <pd/base/job.H>
#include <pd/base/size.H>
#include <pd/base/thr.H>
#include <pd/base/cond.H>
#include <pd/base/config.H>
#include <pd/base/stat.H>
#include <pd/base/stat_items.H>
#include <pd/base/config_enum.H>
#include <pd/base/exception.H>

namespace phantom {

static string_t const pool_label = STRING("pool");

class scheduler_combined_t : public scheduler_t {
	virtual bq_post_activate_t *post_activate() throw();
	virtual bool switch_to(interval_t const &prio);

	class pool_t : public bq_post_activate_t {
		size_t count;

		class queue_t {
			cond_t cond;
			unsigned int waiters;

			bool work;
			bq_heap_t items;

			typedef stat::mmcount_t qcount_t;

			typedef stat::items_t<qcount_t> stat_base_t;

			struct stat_t : stat_base_t {
				inline stat_t() throw() : stat_base_t(STRING("queue")) { }

				inline ~stat_t() throw() { }

				inline qcount_t &qcount() throw() { return item<0>(); }
			};

			stat_t mutable stat;

			inline void init() { stat.init(); }
			inline void stat_print() const { stat.print(); }

			inline void put(bq_heap_t::item_t *item) throw() {
				++stat.qcount();

				cond_t::handler_t handler(cond);

				items.insert(item);

				if(waiters)
					handler.send();
			}

			inline bq_heap_t::item_t *get() throw() {
				bq_heap_t::item_t *item = NULL;

				{
					cond_t::handler_t handler(cond);

					while(!(item = items.head()) && work) {
						++waiters;
						handler.wait();
						--waiters;
					}

					if(item)
						items.remove(item);

					if(waiters && (items.head() || !work))
						handler.send();
				}

				if(item)
					--stat.qcount();

				return item;
			}

			inline void fini() throw() {
				cond_t::handler_t handler(cond);

				work = false;

				if(waiters)
					handler.send();
			}

			inline queue_t() : cond(), waiters(0), work(true), items(), stat() { }
			inline ~queue_t() { assert(!items.head()); assert(!waiters); }

			friend class pool_t;
		};

		queue_t queue;

		class worker_t {
			job_id_t thread;
			pid_t tid;

			typedef stat::count_t acts_t;

			typedef stat::items_t<
				acts_t,
				thr::tstate_t
			> stat_base_t;

			struct stat_t : stat_base_t {
				inline stat_t() throw() : stat_base_t(
					STRING("acts"),
					STRING("tstate")
				) { }

				inline ~stat_t() throw() { }

				inline acts_t &acts() throw() { return item<0>(); }
				inline thr::tstate_t &tstate() throw() { return item<1>(); }
			};

			stat_t mutable stat;

			void loop(queue_t &queue) throw() {
				tid = thr::id;
				thr::tstate = &stat.tstate();

				bq_heap_t::item_t *item;
				while((item = queue.get())) {
					stat.tstate().set(thr::run);

					++stat.acts();
					bq_cont_activate(item->cont);

					stat.tstate().set(thr::idle);
				}

				thr::tstate = NULL;
				tid = 0;
			}

			inline void init(queue_t &queue, string_t const &tname) {
				stat.init();
				thread = job(&worker_t::loop)(*this, queue)->run(tname);
			}

			inline void stat_print() const {
				char buf[16];
				size_t len = ({
					out_t out(buf, sizeof(buf));
					out.print(tid, "05").used();
				});

				stat::ctx_t ctx(str_t(buf, len));
				stat.print();
			}

			inline worker_t() throw() : thread(0), tid(0), stat() { }
			inline ~worker_t() throw() { }

			friend class pool_t;
		};

		worker_t *workers;

	public:
		virtual void operator()(bq_heap_t::item_t *item) {
			queue.put(item);

			bq_cont_deactivate("switching");
		}

		void stat_print() const {
			queue.stat_print();

			{
				stat::ctx_t ctx(CSTR("pool"), 1);

				for(size_t i = 0; i < count; ++i)
					workers[i].stat_print();
			}
		}

		inline pool_t(size_t _count) throw() :
			count(_count), queue(), workers(NULL) { }

		inline void init(string_t const &tname) {
			log::handler_t handler(pool_label);

			queue.init();

			workers = new worker_t[count];

			char const *fmt = log::number_fmt(count);

			size_t i = 0;

			try {
				for(; i < count; ++i) {
					string_t _name = string_t::ctor_t(5).print(i, fmt);
					log::handler_t handler(_name);

					workers[i].init(queue, tname);
				}
			}
			catch(...) {
				while(i--) job_wait(workers[i].thread);

				delete [] workers;
				workers = NULL;

				throw;
			}
		}

		inline void fini() {
			log::handler_t handler(pool_label);

			queue.fini();

			for(size_t i = count; i--;)
				job_wait(workers[i].thread);

			delete [] workers;
			workers = NULL;
		}

		inline ~pool_t() throw() { }
	};

	pool_t pool;

	virtual void init_ext() {
		string_t _tname = string_t::ctor_t(tname.size() + 5)(tname)(CSTR(".pool"));
		pool.init(_tname);
	}
	virtual void stat_print_ext() const { pool.stat_print(); }
	virtual void fini_ext() { pool.fini(); }

public:
	struct config_t : scheduler_t::config_t {
		sizeval_t poolsize;

		inline config_t() throw() : scheduler_t::config_t(), poolsize(16) { }
		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &ptr) const {
			scheduler_t::config_t::check(ptr);

			if(!poolsize)
				config::error(ptr, "poolsize must be a positive number");

			if(poolsize > 512)
				config::error(ptr, "poolsize is too big");
		}
	};

	inline scheduler_combined_t(string_t const &name, config_t const &config) :
		scheduler_t(name, config), pool(config.poolsize) { }

	inline ~scheduler_combined_t() throw() { }
};

namespace scheduler_combined {
config_binding_sname(scheduler_combined_t);
config_binding_parent(scheduler_combined_t, scheduler_t);
config_binding_value(scheduler_combined_t, poolsize);
config_binding_ctor(scheduler_t, scheduler_combined_t);
}

bq_post_activate_t *scheduler_combined_t::post_activate() throw() {
	return &pool;
}

bool scheduler_combined_t::switch_to(interval_t const &prio) {
	if(!bq_thr_set(bq_thr()))
		return bq_success(bq_overload);

	interval_t timeout = prio;
	bq_heap_t::item_t item(&timeout, true);

	pool(&item);

	return bq_success(bq_ok);
}

} // namespace phantom
