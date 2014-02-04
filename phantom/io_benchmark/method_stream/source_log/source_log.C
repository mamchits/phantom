// This file is part of the phantom::io_benchmark::method_stream::source_log module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../source.H"
#include "../../../module.H"

#include <pd/base/config.H>
#include <pd/base/exception.H>
#include <pd/base/in_fd.H>
#include <pd/base/mutex.H>
#include <pd/base/size.H>
#include <pd/base/config.H>

#include <pd/bq/bq_util.H>

#include <unistd.h>
#include <fcntl.h>

namespace phantom { namespace io_benchmark { namespace method_stream {

MODULE(io_benchmark_method_stream_source_log);

#define TAG_LEN (256)

class log_file_t {
	in_fd_t in;
	in_t::ptr_t ptr;
	timeval_t timeval_start;
	bool work;

public:
	inline log_file_t(size_t ibuf_size, int fd) :
		in(ibuf_size, fd), ptr(in),
		timeval_start(timeval::current()), work(true) { }

	inline ~log_file_t() throw() { }

	bool get_request(
		in_segment_t &request, in_segment_t &tag, interval_t &interval_sleep
	);
};

static void error_handler(in_t::ptr_t const &, char const *) {
	throw exception_log_t(log::error, "format error");
}

bool log_file_t::get_request(
	in_segment_t &request, in_segment_t &tag, interval_t &interval_sleep
) {
	if(!work)
		return false;

	try {
		size_t hlen = 0;
		ptr.parse(hlen, &error_handler);

		if(!hlen) {
			work = false;
			return false;
		}

		if(*ptr == ' ') {
			++ptr;
			unsigned long msec = 0;
			ptr.parse(msec, &error_handler);
			interval_t interval_request = msec * interval::millisecond;

			interval_sleep =
				interval_request - (timeval::current() - timeval_start);
		}

		if(*ptr == ' ') {
			in_t::ptr_t tagp = ++ptr;
			size_t limit = TAG_LEN;
			if(!ptr.scan("\n", 1, limit))
				throw exception_log_t(log::error, "format error #1");

			limit = ptr - tagp;
			if(tagp.scan("\t", 1, limit))
				throw exception_log_t(log::error, "format error #2");

			tag = in_segment_t(tagp, ptr - tagp);
		}
		else {
			tag = in_segment_t();
		}

		if(*ptr != '\n')
			throw exception_log_t(log::error, "format error #3");

		++ptr;

		request = in_segment_t(ptr, hlen);

		ptr += hlen;

		if(*ptr != '\n')
			throw exception_log_t(log::error, "format error #4");

		++ptr;

		in.truncate(ptr);
	}
	catch(...) {
		work = false;
		throw;
	}

	return true;
}

class source_log_t : public source_t {
public:
	struct config_t {
		string_t filename;
		sizeval_t ibuf_size;

		inline config_t() throw() : filename(), ibuf_size(sizeval::mega) { }

		inline void check(in_t::ptr_t const &ptr) const {
			if(!filename)
				config::error(ptr, "filename is required");

			if(ibuf_size > sizeval::giga)
				config::error(ptr, "ibuf_size is too big");

			if(ibuf_size < sizeval::kilo)
				config::error(ptr, "ibuf_size is too small");
		}
	};

private:
	mutex_t mutable mutex;
	int fd;
	size_t ibuf_size;
	string_t filename;
	log_file_t *log_file;

	virtual void do_init();
	virtual void do_run() const { }
	virtual void do_stat_print() const { stat.print(); }
	virtual void do_fini();

	virtual bool get_request(in_segment_t &request, in_segment_t &tag) const;

	typedef stat::mminterval_t delta_t;

	typedef stat::items_t<delta_t> stat_base_t;

	struct stat_t : stat_base_t {
		inline stat_t() throw() : stat_base_t(
			STRING("delta")
		) { }

		inline ~stat_t() throw() { }

		inline delta_t &delta() throw() { return item<0>(); }
	};

	stat_t mutable stat;

public:
	inline source_log_t(string_t const &name, config_t const &config) :
		source_t(name), mutex(), fd(-1), ibuf_size(config.ibuf_size),
		filename(config.filename), log_file(NULL), stat() { }

	inline ~source_log_t() throw() { }
};

namespace source_log {
config_binding_sname(source_log_t);
config_binding_value(source_log_t, filename);
config_binding_value(source_log_t, ibuf_size);
config_binding_cast(source_log_t, source_t);
config_binding_ctor(source_t, source_log_t);
}

void source_log_t::do_init() {
	MKCSTR(_filename, filename);

	fd = open(_filename, O_RDONLY, 0);
	if(fd < 0)
		throw exception_sys_t(log::error, errno, "open (%s): %m", _filename);

	log_file = new log_file_t(ibuf_size, fd);

	stat.init();
}

bool source_log_t::get_request(in_segment_t &request, in_segment_t &tag) const {
	interval_t interval_sleep = interval::zero;
	bool res = ({
		mutex_guard_t guard(mutex);

		if(!log_file)
			return false;

		log_file->get_request(request, tag, interval_sleep);
	});

	if(res) {
		if(interval_sleep > interval::zero) {
			stat.delta() = interval::zero;
			if(bq_sleep(&interval_sleep) < 0)
				return false;
		}
		else
			stat.delta() = -interval_sleep;
	}

	return res;
}

void source_log_t::do_fini() {
	log_file_t *_log_file = NULL;
	{
		mutex_guard_t guard(mutex);
		_log_file = log_file;
		log_file = NULL;
	}

	delete _log_file;
	close(fd);
	fd = -1;
}

}}} // namespace phantom::io_benchmark::method_stream
