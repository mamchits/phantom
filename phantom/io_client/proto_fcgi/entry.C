// This file is part of the phantom::io_client::proto_fcgi module.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "entry.I"
#include "instance.I"

#include <pd/base/exception.H>
#include <pd/base/random.H>
#include <pd/base/size.H>

namespace phantom { namespace io_client { namespace proto_fcgi {

void entry_t::instances_t::put(instance_t *instance, size_t ind) {
	(instances[ind - 1] = instance)->ind = ind;
}

instance_t *entry_t::instances_t::get(size_t ind) const {
	assert(ind > 0);
	instance_t *instance = instances[ind - 1];
	assert(instance && instance->ind == ind);
	return instance;
}

bool entry_t::instances_t::rand() {
	if(!rand_cnt) {
		rand_val = random_U();
		rand_cnt = 31;
	}

	bool res = rand_val & 1;
	rand_val >>= 1;
	--rand_cnt;
	return res;
}

void entry_t::instances_t::place(instance_t *instance, size_t i, bool flag) {
	unsigned int rank = instance->rank();

	assert(i > 0);

	size_t j;
	for(; (j = i / 2); i = j) {
		instance_t *_instance = get(j);
		unsigned int _rank = _instance->rank();

		if(_rank <= rank) break;

		put(_instance, i);

		flag = false;
	}

	if(flag) {
		for(; (j = i * 2) <= count; i = j) {
			instance_t *_instance = get(j);
			unsigned int _rank = _instance->rank();

			if(j < count) {
				instance_t *__instance = get(j + 1);
				unsigned int __rank = __instance->rank();

				if(__rank < _rank || (__rank == _rank && rand())) {
					++j;
					_instance = __instance;
				}
			}

			if(rank <= _rank) break;

			put(_instance, i);
		}
	}

	put(instance, i);
}

void entry_t::instances_t::insert(instance_t *instance) {
	assert(instance->ind == 0);
	assert(count < max_count);

	place(instance, ++count, false);
}

void entry_t::instances_t::remove(instance_t *instance) {
	instance_t *_instance = get(count--);

	if(_instance != instance)
		place(_instance, instance->ind, true);

	instance->ind = 0;
}

void entry_t::instances_t::dec_rank(instance_t *instance) {
	assert(instance->trank > 0);
	--instance->trank;

	if(instance->ind > 0)
		place(instance, instance->ind, true);
}

void entry_t::instances_t::inc_rank(instance_t *instance) {
	assert(instance->ind > 0);

	++instance->trank;
	place(instance, instance->ind, true);
}

class entry_t::content_t : public reply_t::content_t {
	http::code_t http_code;
	in_segment_t content_type;
	in_segment_t entity;

	virtual http::code_t code() const throw() { return http_code; }

	virtual void print_header(out_t &out, http::server_t const &) const {
		if(content_type)
			out(CSTR("Content-type:"))(content_type); // ' ' and CRLF inside
	}

	virtual ssize_t size() const throw() { return entity.size(); }

	virtual bool print(out_t &out) const {
		out(entity);
		return true;
	}

	virtual ~content_t() throw() { }

public:
	content_t(in_segment_t const &reply);
};

static bool parse_status(in_segment_t const &str, unsigned short &code) {
	in_t::ptr_t ptr = str;

	while(true) {
		if(!ptr) return false;
		char c = *ptr;
		if(c != ' ' && c != '\t' && c != '\r' && c != '\n') break;
		++ptr;
	}

	if(!ptr.parse(code))
		return false;

	if(!ptr)
		return true;

	char c = *ptr;
	if(c != ' ' && c != '\t' && c != '\r' && c != '\n') return false;

	return true;
}

entry_t::content_t::content_t(in_segment_t const &stdout) :
	http_code(http::code_200), content_type(), entity() {

	in_t::ptr_t ptr = stdout;
	in_t::ptr_t bound = ptr + stdout.size();

	in_t::ptr_t p = ptr;

	http::limits_t limits(0, 16, 64 * sizeval::kilo, 0);
	http::eol_t eol('\r', '\n');
	http::mime_header_t header;

	try {
		header.parse(p, eol, limits);

		in_segment_t const *val = header.lookup(CSTR("content-type"));
		if(val)
			content_type = *val;

		val = header.lookup(CSTR("status"));
		if(val) {
			unsigned short _code;
			if(parse_status(*val, _code))
				http_code = (http::code_t)_code;
		}

		if(p < bound)
			entity = in_segment_t(p, bound - p);
	}
	catch(exception_t const &) {
		throw http::exception_t(http::code_502, "Bad Gateway");
	}
}

bool entry_t::proc(
	request_t const &request, reply_t &reply,
	interval_t *timeout, string_t const &root
) {
	instance_t *instance = ({
		spinlock_guard_t guard(instances_spinlock);
		if(instances.get_count() < quorum)
			return false;

		instance_t *instance = instances.head();
		if(instance->trank >= queue_size)
			return false;

		instances.inc_rank(instance);

		instance;
	});

	bool res = false;

	try {
		//log::handler_t handler(instance->name);

		ref_t<task_t> task = instance->create_task(request, timeout, root);
		if(task) {
			if(task->wait(timeout)) {
				// FIXME: check app_status, proto_code, stderr
				log_debug("got reply as=%u, pc=%u", task->app_status, task->proto_code);

				if(task->stderr.size()) {
					string_t str = string_t(task->stderr);
					log_error("stderr = \"%.*s\"", (int)str.size(), str.ptr());
					throw http::exception_t(http::code_502, "Bad Gateway");
				}

				reply.set(new content_t(task->stdout));
				res = true;
			}
			else {
				instance->abort(task);
			}
		}
	}
	catch(...) { }

	{
		spinlock_guard_t guard(instances_spinlock);
		instances.dec_rank(instance);
	}

	return res;
}

}}} // namespace phantom::io_client::proto_fcgi
