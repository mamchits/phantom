// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "logger.H"

#include "../../io_logger_file.H"

namespace phantom { namespace io_benchmark { namespace method_stream {

class logger_default_t : public logger_t, io_logger_file_t {

	virtual void commit(
		in_segment_t const &request, in_segment_t const &tag, result_t const &res
	);

public:
	struct config_t : logger_t::config_t, io_logger_file_t::config_t {

		inline config_t() throw() :
			logger_t::config_t(), io_logger_file_t::config_t() { }

		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &ptr) const {
			logger_t::config_t::check(ptr);
			io_logger_file_t::config_t::check(ptr);
		}
	};

	inline logger_default_t(string_t const &name, config_t const &config) :
		logger_t(name, config), io_logger_file_t(name, config) { }

	inline ~logger_default_t() throw() { }
};

namespace logger_default {
config_binding_sname(logger_default_t);
config_binding_parent(logger_default_t, logger_t, 1);
config_binding_parent(logger_default_t, io_logger_file_t, 2);
config_binding_ctor(logger_t, logger_default_t);
}

void logger_default_t::commit(
	in_segment_t const &request, in_segment_t const &, result_t const &res
) {
	char header[1024];
	size_t header_len = ({
		out_t out(header, sizeof(header));
		out
			.print(request.size())(' ').print(res.reply.size())(' ')
			.print((res.time_end - res.time_start) / interval_microsecond)(' ')
			.print(res.interval_event / interval_microsecond)(' ')
			.print(res.err).lf()
		;
		out.used();
	});

	size_t iovec_max_cnt = 1 + request.fill(NULL) + 1 + res.reply.fill(NULL) + 1;

	iovec iov[iovec_max_cnt];
	size_t cnt = 0;

	iov[cnt++] = (iovec){ (void *)header, header_len };

	cnt += request.fill(iov + cnt);

	iov[cnt++] = (iovec){ (void *)"\n", 1 };

	cnt += res.reply.fill(iov + cnt);

	iov[cnt++] = (iovec){ (void *)"\n", 1 };

	assert(cnt == iovec_max_cnt);

	writev(iov, cnt);
}

}}} // namespace phantom::io_benchmark::method_stream
