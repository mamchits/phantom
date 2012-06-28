// This file is part of the phantom::io_client::proto_fcgi module.
// Copyright (C) 2011, 2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011, 2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "proto_fcgi.H"
#include "entry.I"
#include "instance.I"

#include "../../module.H"

#include <pd/base/size.H>
#include <pd/base/config.H>

namespace phantom { namespace io_client {

MODULE(io_client_proto_fcgi);

struct proto_fcgi_t::config_t {
	sizeval_t ibuf_size, obuf_size, queue_size, quorum;

	inline config_t() throw() :
		ibuf_size(4 * sizeval_kilo), obuf_size(sizeval_kilo),
		queue_size(16), quorum(1) { }

	inline void check(in_t::ptr_t const &ptr) const {
		if(ibuf_size > sizeval_mega)
			config::error(ptr, "ibuf_size is too big");

		if(ibuf_size < sizeval_kilo)
			config::error(ptr, "ibuf_size is too small");

		if(obuf_size > sizeval_mega)
			config::error(ptr, "obuf_size is too big");

		if(obuf_size < sizeval_kilo)
			config::error(ptr, "obuf_size is too small");

		if(!queue_size)
			config::error(ptr, "queue_slize is zero");

		if(!quorum)
			config::error(ptr, "quorum is zero");
	}

	inline ~config_t() throw() { }
};

namespace proto_fcgi {
config_binding_sname(proto_fcgi_t);
config_binding_value(proto_fcgi_t, ibuf_size);
config_binding_value(proto_fcgi_t, obuf_size);
config_binding_value(proto_fcgi_t, queue_size);
config_binding_value(proto_fcgi_t, quorum);
config_binding_ctor(proto_t, proto_fcgi_t);
}

proto_fcgi_t::prms_t::prms_t(config_t const &config) throw() :
	ibuf_size(config.ibuf_size), obuf_size(config.obuf_size),
	queue_size(config.queue_size), quorum(config.quorum) { }

proto_fcgi_t::proto_fcgi_t(string_t const &, config_t const &config) throw() :
	instances(0), entry(NULL), prms(config) { }

proto_fcgi_t::~proto_fcgi_t() throw() {
	if(entry) delete entry;
}

proto_instance_t *proto_fcgi_t::create_instance(string_t const &name) {
	assert(!entry);
	++instances;

	return new instance_t(*this, name);
}

void proto_fcgi_t::init() {
	entry = new entry_t(prms.queue_size, instances, prms.quorum);
}

void proto_fcgi_t::run() {
	assert(entry);
}

bool proto_fcgi_t::proc(
	request_t const &request, reply_t &reply,
	interval_t *timeout, string_t const &root
) {
	return entry->proc(request, reply, timeout, root);
}

void proto_fcgi_t::stat(out_t &out, bool clear) {
	assert(entry);
	entry->stat(out, clear);
}

}} // namespace phantom::io_client
