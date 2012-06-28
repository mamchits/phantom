// This file is part of the phantom::io_stream::proto_http module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "logger.H"

#include "../../io_logger_file.H"

namespace phantom { namespace io_stream { namespace proto_http {

class logger_default_t :
	public logger_t, io_logger_file_t {

	virtual void commit(request_t const &request, reply_t const &rep) const;

public:
	struct config_t : io_logger_file_t::config_t {
		inline config_t() throw() : io_logger_file_t::config_t() { }

		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &ptr) const {
			io_logger_file_t::config_t::check(ptr);
		}
	};

	inline logger_default_t(string_t const &name, config_t const &config) :
		logger_t(), io_logger_file_t(name, config) { }

	inline ~logger_default_t() throw() { }
};

namespace logger_default {
config_binding_sname(logger_default_t);
config_binding_parent(logger_default_t, io_logger_file_t, 1);
config_binding_ctor(logger_t, logger_default_t);
}

void logger_default_t::commit(
	request_t const &request, reply_t const &reply
) const {
	char buf[128];  // >= 21 + 26 + 22 + 4 + 21 + 21
	size_t len = ({
		out_t out(buf, sizeof(buf));

		out
			.print(request.all.size())('\t')
			.print(request.time, "+dz.")('\t')
			.print(request.remote_addr)('\t')
			.print((uint16_t)reply.code())('\t')
			.print(reply.size())('\t')
			.print((timeval_current() - request.time) / interval_microsecond).lf()
		;
		out.used();
	});

	size_t iovec_max_cnt = 1 + request.all.fill(NULL) + 1;

	iovec iov[iovec_max_cnt];
	size_t cnt = 0;

	iov[cnt++] = (iovec){ (void *)buf, len };

	cnt += request.all.fill(iov + cnt);

	iov[cnt++] = (iovec){ (void *)"\n", 1 };

	assert(cnt == iovec_max_cnt);

	writev(iov, cnt);
}

}}} // namespace phantom::io_stream::proto_http
