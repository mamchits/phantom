// This file is part of the phantom::io_stream::local module.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../io_stream.H"
#include "../../module.H"

#include <pd/base/netaddr_local.H>
#include <pd/base/exception.H>

namespace phantom {

MODULE(io_stream_local);

class io_stream_local_t : public io_stream_t {
	netaddr_local_t netaddr;
	bool defer_accept;

	virtual netaddr_t const &bind_addr() const throw();
	virtual netaddr_t *new_netaddr() const;
	virtual void fd_setup(int fd) const;
	virtual fd_ctl_t const *ctl() const;

public:
	struct config_t : io_stream_t::config_t {
		string_t path;

		inline config_t() throw() :
			io_stream_t::config_t(), path() { }

		inline void check(in_t::ptr_t const &ptr) const {
			io_stream_t::config_t::check(ptr);

			if(!path)
				config::error(ptr, "path is required");

			if(path.size() > netaddr_local_t::max_len())
				config::error(ptr, "path is too long");
		}

		inline ~config_t() throw() { }
	};

	inline io_stream_local_t(string_t const &name, config_t const &config) :
		io_stream_t(name, config), netaddr(config.path.str()) { }

	inline ~io_stream_local_t() throw() { }
};

netaddr_t const &io_stream_local_t::bind_addr() const throw() {
	return netaddr;
}

netaddr_t *io_stream_local_t::new_netaddr() const {
	return new netaddr_local_t;
}

void io_stream_local_t::fd_setup(int /*fd*/) const { }

fd_ctl_t const *io_stream_local_t::ctl() const { return NULL; }

namespace io_stream_local {
config_binding_sname(io_stream_local_t);
config_binding_value(io_stream_local_t, path);
config_binding_parent(io_stream_local_t, io_stream_t, 1);
config_binding_ctor(io_t, io_stream_local_t);
}

} // namespace phantom
