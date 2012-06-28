// This file is part of the phantom program.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This program may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "scheduler.H"

#include <pd/bq/bq_heap.H>
#include <pd/bq/bq_job.H>

#include <pd/base/size.H>
#include <pd/base/config.H>
#include <pd/base/config_enum.H>
#include <pd/base/exception.H>

#include <unistd.h>

namespace phantom {

class scheduler_combined_t : public scheduler_t {
	virtual bq_cont_activate_t &activate() throw();
	virtual bool switch_to(interval_t const &prio);
	virtual void init_ext();
	virtual void stat_ext(out_t &out, bool clear);
	virtual void fini_ext();

	class pool_t : public bq_cont_activate_t {
		typedef thr::spinlock_t mutex_t;
		typedef thr::spinlock_guard_t mutex_guard_t;

		size_t num;

		class queue_t {
			pthread_cond_t cond;
			pthread_mutex_t mutex;
			unsigned int waiters;

			bool work;
			bq_heap_t items;

		public:
			inline void put(bq_heap_t::item_t *item) throw() {
				pthread_mutex_lock(&mutex);
				items.insert(item);

				if(waiters)
					pthread_cond_signal(&cond);

				pthread_mutex_unlock(&mutex);
			}

			inline bq_heap_t::item_t *get() throw() {
				bq_heap_t::item_t *item;

				pthread_mutex_lock(&mutex);

				while(!(item = items.head()) && work) {
					++waiters;
					pthread_cond_wait(&cond, &mutex);
					--waiters;
				}

				if(item)
					items.remove(item);

				if(waiters && (items.head() || !work))
					 pthread_cond_signal(&cond);

				pthread_mutex_unlock(&mutex);

				return item;
			}

			inline void fini() throw() {
				pthread_mutex_lock(&mutex);

				work = false;

				if(waiters)
					pthread_cond_signal(&cond);

				pthread_mutex_unlock(&mutex);
			}

			inline queue_t() :
				waiters(0), work(true), items() {

				pthread_cond_init(&cond, NULL);
				pthread_mutex_init(&mutex, NULL);
			}

			inline ~queue_t() {
				assert(!items.head());
				assert(!waiters);

				pthread_cond_destroy(&cond);
				pthread_mutex_destroy(&mutex);
			}
		};

		queue_t queue;

		pthread_t *threads;

		mutex_t stat_mutex;
		size_t wcount, wcount_max, wcount_max_total;

		virtual void operator()(bq_heap_t::item_t *item, bq_err_t err) {
			enqueue(item, err);
		}

	public:
		inline void enqueue(bq_heap_t::item_t *item, bq_err_t err) {
			bq_cont_stat_update(item->cont, wait_ext);
			bq_cont_set_msg(item->cont, err);

			{
				mutex_guard_t guard(stat_mutex);
				if(++wcount > wcount_max) wcount_max = wcount;
			}
			queue.put(item);
		}

		void proc() throw() {
			bq_heap_t::item_t *item;
			while((item = queue.get())) {
				{
					mutex_guard_t guard(stat_mutex);
					--wcount;
				}

				bq_cont_activate(item->cont);
			}
		}

		void stat(out_t &out, bool clear) {
			size_t c, cmax;
			{
				mutex_guard_t guard(stat_mutex);
				c = wcount;
				if(wcount_max > wcount_max_total) wcount_max_total = wcount_max;
				cmax = clear ? wcount_max : wcount_max_total;
				wcount_max = wcount;
			}

			out(CSTR("Queue len: ")).print(c)(' ')('(').print(cmax)(')').lf();
		}

		inline pool_t(size_t _num) throw() :
			num(_num), queue(), threads(NULL),
			stat_mutex(), wcount(0), wcount_max(0), wcount_max_total(0) { }

		inline void init(string_t const &name) {
			threads = new pthread_t[num];

			try {
				size_t i = 0;

				try {
					for(; i < num; ++i) {
						string_t _name = string_t::ctor_t(name.size() + 5 + 3 + 1)
							(name)(CSTR(".pool[")).print(i)(']')
						;

						bq_job_t<typeof(&pool_t::proc)>::create(
							_name, &threads[i], NULL, *this, &pool_t::proc
						);
					}
				}
				catch(...) {
					while(i--) pthread_join(threads[i], NULL);
					throw;
				}
			}
			catch(...) {
				delete threads;
				threads = NULL;

				throw;
			}
		}

		inline void fini() {
			queue.fini();

			for(size_t i = num; i--;) {
				errno = pthread_join(threads[i], NULL);
				if(errno)
					log_error("pthread_join: %m");
			}

			delete threads;
			threads = NULL;
		}

		inline ~pool_t() throw() { }
	};

	pool_t pool;

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
config_binding_parent(scheduler_combined_t, scheduler_t, 1);
config_binding_value(scheduler_combined_t, poolsize);
config_binding_ctor(scheduler_t, scheduler_combined_t);
}

namespace scheduler_combined {

class switch_item_t : public bq_heap_t::item_t {
	virtual void attach() throw() { }
	virtual void detach() throw() { }

public:
	inline switch_item_t(interval_t *timeout) throw() :
		bq_heap_t::item_t(timeout) { }
};

} // scheduler_combined

bq_cont_activate_t &scheduler_combined_t::activate() throw() {
	return pool;
}

bool scheduler_combined_t::switch_to(interval_t const &prio) {
	if(!bq_thr_set(bq_thr()))
		return bq_success(bq_overload);

	interval_t timeout = prio;
	scheduler_combined::switch_item_t item(&timeout);

	pool.enqueue(&item, bq_ok);

	return bq_success(bq_cont_deactivate("switching", wait_ext));
}

void scheduler_combined_t::init_ext() {
	pool.init(name);
}

void scheduler_combined_t::stat_ext(out_t &out, bool clear) {
	pool.stat(out, clear);
}

void scheduler_combined_t::fini_ext() {
	pool.fini();
}

} // namespace phantom
