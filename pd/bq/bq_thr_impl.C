// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_thr_impl.I"
#include "bq_util.H"

#include <pd/base/exception.H>
#include <pd/base/thr.H>

#include <unistd.h>
#include <sys/epoll.h>

namespace pd {

bq_thr_t::impl_t::impl_t(
	size_t _maxevs, interval_t _timeout, bq_cont_count_t &_cont_count,
	bq_post_activate_t *_post_activate
) :
	cont_count(_cont_count), post_activate(_post_activate),
	thread(0), tid(0),
	maxevs(_maxevs), timeout(_timeout), stat(), entry() {

	efd = epoll_create(maxevs);
	if(efd < 0)
		throw exception_sys_t(log::error, errno, "bq_thr_t::impl_t::impl_t, epoll_create: %m");

	if(::pipe(sig_fds) < 0)
		throw exception_sys_t(log::error, errno, "bq_thr_t::impl_t::impl_t, pipe: %m");

	bq_fd_setup(sig_fds[0]);
	bq_fd_setup(sig_fds[1]);

	epoll_event ev;
	ev.events = POLLIN;
	ev.data.ptr = NULL;

	if(epoll_ctl(efd, EPOLL_CTL_ADD, sig_fds[0], &ev) < 0)
		throw exception_sys_t(log::error, errno, "bq_thr_t::impl_t::impl_t, epoll_ctl, add: %m");
}

__thread bq_thr_t::impl_t *bq_thr_t::impl_t::current;

bool bq_thr_t::impl_t::work = true;

void bq_thr_t::impl_t::poke() const throw() {
	if(this == current)
		return;

	char c = 0;
	if(::write(sig_fds[1], &c, 1) < 0) {
		if(errno != EAGAIN)
			log_error("bq_thr_t::impl_t::poke: %m");
	}
}

bq_thr_t::impl_t::~impl_t() throw() {
	epoll_event ev;
	ev.events = 0;
	ev.data.ptr = NULL;

	if(epoll_ctl(efd, EPOLL_CTL_DEL, sig_fds[0], &ev) < 0)
		log_error("bq_thr_t::impl_t::~impl_t, epoll_ctl, del: %m");

	if(::close(efd) < 0)
		log_error("bq_thr_t::impl_t::~impl_t, close: %m");

	if(::close(sig_fds[0]) < 0)
		log_error("bq_thr_t::impl_t::~impl_t, close (sig_fds[0]): %m");

	if(::close(sig_fds[1]) < 0)
		log_error("bq_thr_t::impl_t::~impl_t, close (sig_fds[1]): %m");
}

class poll_item_t : public bq_thr_t::impl_t::item_t {
	int fd;
	short int &events;

	virtual void attach() throw();
	virtual void detach() throw();

public:
	inline poll_item_t(
		int _fd, short int &_events, interval_t *_timeout
	) throw() :
		item_t(_timeout, false), fd(_fd), events(_events) { }

	inline ~poll_item_t() throw() { }

	friend class bq_thr_t::impl_t;
};

void poll_item_t::attach() throw() {
	epoll_event ev;
	ev.events = events | EPOLLET | EPOLLONESHOT;
	ev.data.ptr = this;

	events = 0;

	if(epoll_ctl(impl->efd, EPOLL_CTL_ADD, fd, &ev) < 0) {
		log_error("poll_item_t::attach, epoll_ctl, add: %m");
		fatal("impossible to continue");
	}
}

void poll_item_t::detach() throw() {
	epoll_event ev;
	ev.events = 0;
	ev.data.ptr = NULL;

	if(epoll_ctl(impl->efd, EPOLL_CTL_DEL, fd, &ev) < 0) {
		log_error("poll_item_t::detach, epoll_ctl, del: %m");
		fatal("impossible to continue");
	}
}

bq_err_t bq_do_poll(
	int fd, short int &events, interval_t *timeout, char const *where
) {
	poll_item_t item(fd, events, timeout);

	return item.suspend(where);
}

void bq_thr_t::impl_t::loop() {
	tid = thr::id;
	current = this;
	thr::tstate = &stat.tstate();

	epoll_event evs[maxevs];
	int timeoit_msec = timeout / interval::millisecond;

	struct heaps_t {
		bq_heap_t common, ready;

		inline heaps_t() : common(), ready() { }

		inline void insert(bq_heap_t::item_t *item) {
			bq_heap_t *heap = item->heap;
			if(heap) {
				if(item->ready) {
					if(heap == &ready) return;
				}
				else {
					if(heap == &common) return;
				}

				heap->remove(item);
			}

			(item->ready ? ready : common).insert(item);
		}

		inline void remove(bq_heap_t::item_t *item) {
			bq_heap_t *heap = item->heap;
			if(heap)
				heap->remove(item);
		}

		inline bq_heap_t::item_t *head(timeval_t const &now, bool work) {
			bq_heap_t::item_t *citem = common.head();
			bq_heap_t::item_t *ritem = ready.head();

			if(work) {
				if(citem && citem->time_to >= now)
					citem = NULL;

				if(ritem) {
					if(!citem || citem->time_to >= ritem->time_to)
						return ritem;
				}
			}
			else {
				if(ritem)
					return ritem;
			}

			return citem;
		}
	} heaps;

	while(work || bq_cont_count()) {
		int n = epoll_wait(efd, evs, maxevs, timeoit_msec);
		if(n < 0) {
			if(errno != EINTR)
				throw exception_sys_t(log::error, errno, "bq_thr_t::impl_t::loop, epoll_wait: %m");
			n = 0;
		}

		stat.tstate().set(thr::run);

		for(int i = 0; i < n; ++i) {
			void *ptr = evs[i].data.ptr;
			if(ptr) {
				poll_item_t *item = (poll_item_t *)ptr;

				item->events = evs[i].events & ~(EPOLLET | EPOLLONESHOT);

					// may be "item->ready = true; heaps.insert(item);"
				entry.set_ready(item);
			}
			else {
				char buf[1024];
				if(::read(sig_fds[0], buf, sizeof(buf)) < 0) {
					if(errno != EAGAIN)
						log_error("bq_thr_t::impl_t::clear: %m");
				}
			}
		}

		while(true) {
			bq_heap_t::item_t *item = NULL;
			while((item = entry.remove()))
				heaps.insert(item);

			timeval_t time = timeval::current();

			if(!(item = heaps.head(time, work)))
				break;

			heaps.remove(item);

			++stat.acts();

			if(item->ready) {
				if(item->timeout)
					*item->timeout =
						item->time_to > time ? item->time_to - time : interval::zero;

				item->err = bq_ok;
			}
			else {
				if(item->timeout) *item->timeout = interval::zero;

				item->err = work ? bq_timeout : bq_not_available;
			}

			bq_cont_activate(item->cont);
		}

		stat.tstate().set(thr::idle);
	}

	thr::tstate = NULL;
	current = NULL;
	tid = 0;
}

void bq_thr_t::impl_t::init(string_t const &tname) {
	stat.init();

	thread = job(&impl_t::loop)(*this)->run(tname);
}

void bq_thr_t::impl_t::fini() {
	poke();
	job_wait(thread);
	thread = 0;
}

} // namespace pd
