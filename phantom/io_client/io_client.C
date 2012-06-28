// This file is part of the phantom::io_client module.
// Copyright (C) 2010-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2012, YANDEX LLC.
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

private:
	virtual void init();
	virtual void run();
	virtual void stat(out_t &out, bool clear);
	virtual void fini();

	link_t *links;
	proto_t &proto;

public:
	struct config_t : io_t::config_t {
		config_binding_type_ref(links_t);
		config_binding_type_ref(proto_t);

		sizeval_t ibuf_size, obuf_size;
		interval_t conn_timeout;

		config::list_t<config::objptr_t<links_t> > links;
		config::objptr_t<proto_t> proto;

		inline config_t() throw() :
			io_t::config_t(),
			conn_timeout(interval_second) { }

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
config_binding_value(io_client_t, links);
config_binding_value(io_client_t, proto);
config_binding_parent(io_client_t, io_t, 1);
config_binding_ctor(io_t, io_client_t);
}

io_client_t::io_client_t(string_t const &name, config_t const &config) :
	io_t(name, config), links(NULL), proto(*config.proto) {

	try {
		for(typeof(config.links.ptr()) ptr = config.links; ptr; ++ptr)
			ptr.val()->create(links, name, config.conn_timeout, proto);
	}
	catch(...) {
		while(links) delete links;
		throw;
	}
}

io_client_t::~io_client_t() throw() {
	while(links) delete links;
}

void io_client_t::init() {
	proto.init();
}

void io_client_t::run() {
	links->run();
	proto.run();
}

void io_client_t::stat(out_t &out, bool clear) {
	links->stat(out, clear);
	proto.stat(out, clear);
}

void io_client_t::fini() { }

} // namspace phantom
