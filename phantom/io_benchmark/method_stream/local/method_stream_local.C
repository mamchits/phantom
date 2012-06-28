// This file is part of the phantom::io_benchmark::method_stream::local module.
// Copyright (C) 2011, 2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011, 2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../method_stream.H"

#include "../../../module.H"

#include <pd/bq/bq_util.H>

#include <pd/base/config_enum.H>
#include <pd/base/netaddr_local.H>
#include <pd/base/fd_tcp.H>
#include <pd/base/exception.H>

#include <unistd.h>

namespace phantom { namespace io_benchmark {

MODULE(io_benchmark_method_stream_local);

struct method_stream_local_t : public method_stream_t {
	netaddr_local_t netaddr;

	virtual netaddr_t const &dest_addr() const;
	virtual void bind(int fd) const;
	virtual fd_ctl_t const *ctl() const throw() { return NULL; }

public:
	struct config_t : method_stream_t::config_t {
		string_t path;

		inline config_t() throw() :
			method_stream_t::config_t(), path() { }

		inline void check(in_t::ptr_t const &ptr) const {
			method_stream_t::config_t::check(ptr);

			if(!path)
				config::error(ptr, "path is required");

			if(path.size() > netaddr_local_t::max_len())
				config::error(ptr, "path is too long");
		}
	};

	inline method_stream_local_t(string_t const &name, config_t const &config) :
		method_stream_t(name, config), netaddr(config.path.str()) { }

	inline ~method_stream_local_t() throw() { }
};

netaddr_t const &method_stream_local_t::dest_addr() const {
	return netaddr;
}

void method_stream_local_t::bind(int /*fd*/) const { }

namespace method_stream_local {
config_binding_sname(method_stream_local_t);
config_binding_value(method_stream_local_t, path);
config_binding_parent(method_stream_local_t, method_stream_t, 1);
config_binding_ctor(method_t, method_stream_local_t);
}

}} // namespace phantom::io_benchmark
