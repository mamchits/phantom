// This file is part of the phantom::io_client::proto_none module.
// Copyright (C) 2010-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2014, YANDEX LLC.
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
		ibuf_size(4 * sizeval::kilo), obuf_size(sizeval::kilo),
		queue_size(16), quorum(1),
		out_timeout(interval::second), in_timeout(interval::second) { }

	inline void check(in_t::ptr_t const &ptr) const {
		if(ibuf_size > sizeval::mega)
			config::error(ptr, "ibuf_size is too big");

		if(ibuf_size < sizeval::kilo)
			config::error(ptr, "ibuf_size is too small");

		if(obuf_size > sizeval::mega)
			config::error(ptr, "obuf_size is too big");

		if(obuf_size < sizeval::kilo)
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
config_binding_cast(proto_none_t, proto_t);
config_binding_ctor(proto_t, proto_none_t);
}

proto_none_t::prms_t::prms_t(config_t const &config) throw() :
	ibuf_size(config.ibuf_size), obuf_size(config.obuf_size),
	out_timeout(config.out_timeout), in_timeout(config.in_timeout),
	queue_size(config.queue_size), quorum(config.quorum) { }

proto_none_t::proto_none_t(string_t const &name, config_t const &config) throw() :
	proto_t(name), instances(0), entry(NULL), prms(config) { }

proto_none_t::~proto_none_t() throw() {
	if(entry) delete entry;
}

proto_t::instance_t *proto_none_t::create_instance(unsigned int _rank) {
	assert(!entry);
	++instances;

	return new instance_t(*this, _rank);
}

void proto_none_t::do_init() {
	entry = new entry_t(prms.queue_size, instances, prms.quorum);
	entry->init();
}

void proto_none_t::do_run() const {
	assert(entry);
	entry->run();
}

void proto_none_t::do_stat_print() const {
	assert(entry);
	entry->stat_print();
}

void proto_none_t::do_fini() { }

void proto_none_t::put_task(ref_t<task_t> &task) const {
	assert(entry);
	entry->put_task(task);
}

}} // namespace phantom::io_client
