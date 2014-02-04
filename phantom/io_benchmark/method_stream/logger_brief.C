// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "logger.H"

namespace phantom { namespace io_benchmark { namespace method_stream {

class logger_brief_t : public logger_t {

#undef unix

public:
	enum time_format_t {
		native = 0,
		unix = 1
	};

private:
	time_format_t time_format;

	virtual void commit(
		in_segment_t const &request, in_segment_t const &tag, result_t const &res
	) const;

public:
	struct config_t : logger_t::config_t {
		config::enum_t<time_format_t> time_format;

		inline config_t() throw() :
			logger_t::config_t(), time_format(native) { }

		inline ~config_t() throw() { }

		inline void check(in_t::ptr_t const &ptr) const {
			logger_t::config_t::check(ptr);
		}
	};

	inline logger_brief_t(string_t const &name, config_t const &config) :
		logger_t(name, config), time_format(config.time_format) { }

	inline ~logger_brief_t() throw() { }
};

namespace logger_brief {
config_binding_sname(logger_brief_t);
config_binding_value(logger_brief_t, time_format);
config_binding_parent(logger_brief_t, logger_t);
config_binding_ctor(logger_t, logger_brief_t);

config_enum_internal_sname(logger_brief_t, time_format_t);
config_enum_internal_value(logger_brief_t, time_format_t, native);
config_enum_internal_value(logger_brief_t, time_format_t, unix);
}

void logger_brief_t::commit(
	in_segment_t const &, in_segment_t const &tag, result_t const &res
) const {
	char buf[1024 + tag.size()];

	size_t size = ({
		out_t out(buf, sizeof(buf));
		if(time_format == native) {
			out.print(res.time_start, "+dz.")('\t');
		}
		else {
			interval_t t = res.time_start - timeval::unix_origin;
			if(t < interval::zero) t = interval::zero; // Don't wanna play with sign

			out
				.print(t / interval::second)('.')
				.print((t % interval::second) / interval::millisecond, "03")('\t')
			;
		}
		out
			(tag)('\t')
			.print((res.time_end - res.time_start) / interval::microsecond)('\t')
			.print((res.time_conn - res.time_start) / interval::microsecond)('\t')
			.print((res.time_send - res.time_conn) / interval::microsecond)('\t')
			.print((res.time_recv - res.time_send) / interval::microsecond)('\t')
			.print((res.time_end - res.time_recv) / interval::microsecond)('\t')
			.print(res.interval_event / interval::microsecond)('\t')
			.print(res.size_out)('\t')
			.print(res.size_in)('\t')
			.print(res.err)('\t')
			.print(res.proto_code)('\n')
		;

		out.used();
	});

	write(buf, size);
}

}}} // namespace phantom::io_benchmark::method_stream
