// This file is part of the phantom::io_client module.
// Copyright (C) 2010-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "link.H"

#include "../io.H"
#include "../module.H"

#include <pd/base/config_list.H>

namespace phantom {

MODULE(io_client);

class io_client_t : public io_t {
public:
	typedef io_client::link_t link_t;
	typedef io_client::links_t links_t;
	typedef io_client::proto_t proto_t;
	typedef io_client::pool_t pool_t;

private:
	virtual void init();
	virtual void run() const;
	virtual void stat_print() const;
	virtual void fini();

	proto_t &proto;

	struct pools_t {
		size_t size;
		pool_t *items;

		inline pools_t() : size(0), items(NULL) { }
		inline pools_t(pools_t const &) = delete;
		inline pools_t &operator=(pools_t const &) = delete;
		inline ~pools_t() throw() { delete [] items; }

		inline void setup(size_t _size) {
			delete [] items;
			items = new pool_t[size = _size];
		}

		inline pool_t &operator[](size_t ind) { return items[ind]; }

		inline void init() {
			for(size_t i = 0; i < size; ++i) items[i].init();
		}

		inline void run(scheduler_t const &scheduler) const {
			for(size_t i = 0; i < size; ++i) items[i].run(scheduler);
		}

		inline void stat_print() const {
			for(size_t i = 0; i < size; ++i) items[i].stat_print();
		}
	};

	pools_t pools;

public:
	struct config_t : io_t::config_t {
		config_binding_type_ref(links_t);
		config_binding_type_ref(proto_t);

		sizeval_t ibuf_size, obuf_size;
		interval_t conn_timeout;
		config::enum_t<log::level_t> remote_errors;

		config::list_t<config::objptr_t<links_t>> links;
		config::objptr_t<proto_t> proto;

		inline config_t() throw() :
			io_t::config_t(),
			conn_timeout(interval::second), remote_errors(log::error) { }

		inline void check(in_t::ptr_t const &ptr) const {
			io_t::config_t::check(ptr);

			if(!proto)
				config::error(ptr, "proto is required");
		}

		inline ~config_t() throw() { }
	};

	io_client_t(string_t const &name, config_t const &config);
	~io_client_t() throw();
};

namespace io_client {
config_binding_sname(io_client_t);
config_binding_type(io_client_t, links_t);
config_binding_type(io_client_t, proto_t);
config_binding_value(io_client_t, conn_timeout);
config_binding_value(io_client_t, remote_errors);
config_binding_value(io_client_t, links);
config_binding_value(io_client_t, proto);
config_binding_parent(io_client_t, io_t);
config_binding_ctor(io_t, io_client_t);
}

io_client_t::io_client_t(string_t const &name, config_t const &config) :
	io_t(name, config), proto(*config.proto), pools() {

	size_t size = 0;
	for(typeof(config.links._ptr()) ptr = config.links; ptr; ++ptr)
		++size;

	pools.setup(size);

	size_t i = 0;

	for(typeof(config.links._ptr()) ptr = config.links; ptr; ++ptr)
		ptr.val()->create(pools[i++], config.conn_timeout, config.remote_errors, proto);
}

io_client_t::~io_client_t() throw() { }

void io_client_t::init() {
	proto.init(name);
	pools.init();
}

void io_client_t::run() const {
	pools.run(scheduler);
	proto.run(name);
}

void io_client_t::stat_print() const {
	proto.stat_print(name);

	stat::ctx_t ctx(CSTR("links"), 1, pools.size);
	pools.stat_print();
}

void io_client_t::fini() { }

} // namspace phantom
