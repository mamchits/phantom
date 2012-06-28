// This file is part of the phantom::io_stream module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/io.H>

#include <pd/base/size.H>
#include <pd/base/time.H>
#include <pd/base/log.H>
#include <pd/base/netaddr.H>
#include <pd/base/fd.H>
#include <pd/base/config_enum.H>

#pragma GCC visibility push(default)

namespace phantom {

namespace io_stream {
class transport_t;
class proto_t;
class acl_t;
}

class io_stream_t : public io_t {
public:
	typedef io_stream::transport_t transport_t;
	typedef io_stream::proto_t proto_t;
	typedef io_stream::acl_t acl_t;

private:
	virtual netaddr_t const &bind_addr() const throw() = 0;
	virtual netaddr_t *new_netaddr() const = 0;
	virtual void fd_setup(int fd) const = 0;
	virtual fd_ctl_t const *ctl() const = 0;

	int fd;
	size_t listen_backlog;
	bool reuse_addr;
	size_t ibuf_size, obuf_size;
	interval_t timeout;
	interval_t keepalive;
	interval_t force_poll;
	transport_t const &transport;
	proto_t &proto;
	bool multiaccept;
	scheduler_t *aux_scheduler;
	log::level_t remote_errors;

	virtual void init();
	virtual void run();
	virtual void stat(out_t &out, bool clear);
	virtual void fini();

	void loop(int fd, bool conswitch) const;
	void conn_proc(int fd, netaddr_t *netaddr) const;

public:
	struct config_t : io_t::config_t {
		config_binding_type_ref(acl_t);
		config_binding_type_ref(transport_t);
		config_binding_type_ref(proto_t);

		sizeval_t listen_backlog;
		config::enum_t<bool> reuse_addr;
		sizeval_t ibuf_size, obuf_size;
		interval_t timeout;
		interval_t keepalive;
		interval_t force_poll;
		config::objptr_t<transport_t> transport;
		config::objptr_t<proto_t> proto;
		config::enum_t<bool> multiaccept;
		config::objptr_t<scheduler_t> aux_scheduler;
		config::enum_t<log::level_t> remote_errors;

		config_t() throw();
		void check(in_t::ptr_t const &ptr) const;
		inline ~config_t() throw() { }
	};

protected:
	io_stream_t(string_t const &name, config_t const &config);
	~io_stream_t() throw();
};

} // namespace phantnom

#pragma GCC visibility pop
