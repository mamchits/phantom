// This file is part of the phantom::io_client::proto_none module.
// Copyright (C) 2010-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "proto_none.H"
#include "entry.I"
#include "instance.I"

#include "../../module.H"

#include <pd/base/size.H>
#include <pd/base/config.H>

namespace phantom { namespace io_client {

MODULE(io_client_proto_none);

struct proto_none_t::config_t {
	sizeval_t ibuf_size, obuf_size, queue_size, quorum;
	interval_t out_timeout, in_timeout;

	inline config_t() throw() :
		ibuf_size(4 * sizeval_kilo), obuf_size(sizeval_kilo),
		queue_size(16), quorum(1),
		out_timeout(interval_second), in_timeout(interval_second) { }

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

namespace proto_none {
config_binding_sname(proto_none_t);
config_binding_value(proto_none_t, ibuf_size);
config_binding_value(proto_none_t, obuf_size);
config_binding_value(proto_none_t, queue_size);
config_binding_value(proto_none_t, quorum);
config_binding_value(proto_none_t, out_timeout);
config_binding_value(proto_none_t, in_timeout);
config_binding_ctor(proto_t, proto_none_t);
}

proto_none_t::prms_t::prms_t(config_t const &config) throw() :
	ibuf_size(config.ibuf_size), obuf_size(config.obuf_size),
	out_timeout(config.out_timeout), in_timeout(config.in_timeout),
	queue_size(config.queue_size), quorum(config.quorum) { }

proto_none_t::proto_none_t(string_t const &, config_t const &config) throw() :
	instances(0), entry(NULL), prms(config) { }

proto_none_t::~proto_none_t() throw() {
	if(entry) delete entry;
}

proto_instance_t *proto_none_t::create_instance(string_t const &/*name*/) {
	assert(!entry);
	++instances;

	return new instance_t(*this);
}

void proto_none_t::init() {
	entry = new entry_t(prms.queue_size, instances, prms.quorum);
}

void proto_none_t::run() {
	assert(entry);
	entry->run();
}

void proto_none_t::put_task(ref_t<task_t> &task) const {
	entry->put_task(task);
}

void proto_none_t::stat(out_t &out, bool clear) {
	assert(entry);
	entry->stat(out, clear);
}

}} // namespace phantom::io_client
