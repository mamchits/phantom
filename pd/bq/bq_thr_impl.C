// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_thr_impl.I"
#include "bq_util.H"

#include <pd/base/exception.H>

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/syscall.h>

namespace pd {

struct bq_thr_t::impl_t::stat_t {
	interval_t time_wait;
	interval_t time_run;

	inline stat_t() throw() :
		time_wait(interval_zero), time_run(interval_zero) { }

	inline ~stat_t() throw() { }

	inline stat_t &operator+=(stat_t const &stat) throw() {
		time_wait += stat.time_wait;
		time_run += stat.time_run;

		return *this;
	}

	inline void print(pid_t tid, out_t &out) {
		out
			(CSTR("thread ")).print(tid)
			(CSTR(": wait: ")).print(time_wait)
			(CSTR(", run: ")).print(time_run).lf()
		;
	}
};

bq_thr_t::impl_t::impl_t(
	string_t const &_name, size_t _maxevs, interval_t _timeout,
	bq_cont_count_t &_cont_count, bq_cont_activate_t &_activate
) :
	cont_count(_cont_count), activate(_activate), mutex(),
	name(_name), thread(0), tid(0),
	maxevs(_maxevs), timeout(_timeout), time(timeval_current()) {

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

	stat = new stat_t;

	try {
		stat_sum = new stat_t;
	}
	catch(...) {
		delete stat;
		throw;
	}
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

	delete stat_sum;
	delete stat;
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
		item_t(_timeout), fd(_fd), events(_events) { }

	inline ~poll_item_t() throw() { }

	friend class bq_thr_t::impl_t;
};

void poll_item_t::attach() throw() {
	epoll_event ev;
	ev.events = events | EPOLLET | EPOLLONESHOT;
	ev.data.ptr = this;

	events = 0;

	if(epoll_ctl(impl->efd, EPOLL_CTL_ADD, fd, &ev) < 0)
		log_error("poll_item_t::attach, epoll_ctl, add: %m");
}

void poll_item_t::detach() throw() {
	epoll_event ev;
	ev.events = 0;
	ev.data.ptr = NULL;

	if(epoll_ctl(impl->efd, EPOLL_CTL_DEL, fd, &ev) < 0)
		log_error("poll_item_t::detach, epoll_ctl, del: %m");
}

bq_err_t bq_do_poll(
	int fd, short int &events, interval_t *timeout, char const *where
) {
	poll_item_t item(fd, events, timeout);

	return item.suspend(false, where);
}

void bq_thr_t::impl_t::loop() {
	tid = syscall(SYS_gettid);
	current = this;

	epoll_event evs[maxevs];

	while(work || bq_cont_count()) {
		{
			timeval_t _time = timeval_current();
			{
				mutex_guard_t guard(mutex);
				stat->time_run += (_time - time);
			}
			time = _time;
		}

		int n = epoll_wait(efd, evs, maxevs, timeout / interval_millisecond);
		if(n < 0) {
			if(errno != EINTR)
				throw exception_sys_t(log::error, errno, "bq_thr_t::impl_t::loop, epoll_wait: %m");
			n = 0;
		}

		for(int i = 0; i < n; ++i) {
			void *ptr = evs[i].data.ptr;
			if(ptr) {
				poll_item_t *item = (poll_item_t *)ptr;

				item->events = evs[i].events & ~(EPOLLET | EPOLLONESHOT);

				set_ready(item);
			}
			else {
				char buf[1024];
				if(::read(sig_fds[0], buf, sizeof(buf)) < 0) {
					if(errno != EAGAIN)
						log_error("bq_thr_t::impl_t::clear: %m");
				}
			}
		}

		{
			timeval_t _time = timeval_current();
			{
				mutex_guard_t guard(mutex);
				stat->time_wait += (_time - time);
			}
			time = _time;
		}

		while(true) {
			bq_heap_t::item_t *citem = NULL;
			bq_heap_t::item_t *ritem = NULL;

			{
				mutex_guard_t guard(mutex);

				citem = common.head();
				ritem = ready.head();
			}

			bq_heap_t::item_t *item = NULL;

			if(citem && (citem->time_to < time || !work)) {
				if(ritem)
					item = (citem->time_to < ritem->time_to) ? citem : ritem;
				else
					item = citem;
			}
			else {
				if(ritem)
					item = ritem;
				else
					break;
			}

			item->detach();

			bool ready = ({
				mutex_guard_t guard(mutex);
				remove(item);
			});

			if(ready) {
				if(item->timeout)
					*item->timeout =
						item->time_to > time ? item->time_to - time : interval_zero;

				activate(item, bq_ok);
			}
			else {
				if(item->timeout) *item->timeout = interval_zero;

				activate(item, work ? bq_timeout : bq_not_available);
			}
		}
	}

	current = NULL;
}

void bq_thr_t::impl_t::stat_print(out_t &out, bool clear) {
	stat_t *stat_new = new stat_t;

	{
		mutex_guard_t guard(mutex);
		stat_t *stat_tmp = stat;
		stat = stat_new;
		stat_new = stat_tmp;
	}

	(*stat_sum) += *(stat_new);

	(clear ? stat_new : stat_sum)->print(tid, out);

	delete stat_new;
}

} // namespace pd
