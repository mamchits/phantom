// This file is part of the phantom::io_benchmark::method_stream::source_random module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../source.H"
#include "../../../module.H"

#include <pd/base/config_list.H>
#include <pd/base/random.H>
#include <pd/base/thr.H>
#include <pd/base/size.H>
#include <pd/base/config.H>

namespace phantom { namespace io_benchmark { namespace method_stream {

MODULE(io_benchmark_method_stream_source_random);

class source_random_t : public source_t {
	class count_t {
		unsigned int count;
		thr::spinlock_t count_spin;

	public:
		inline count_t(unsigned int _count) : count(_count) { }

		inline bool more() {
			thr::spinlock_guard_t _guard(count_spin);
			if(!count) return false;
			--count;
			return true;
		}
	};

public:
	struct config_t {
		config::list_t<string_t> requests;
		config::list_t<string_t> headers;
		sizeval_t tests;

		inline config_t() throw() :
			requests(), headers(), tests(sizeval_kilo) { }

		inline void check(in_t::ptr_t const &ptr) const {
			if(tests >= (4 * sizeval_giga))
				config::error(ptr, "tests is too big");

			if(!requests)
				config::error(ptr, "request list is empty");
		}
	};

private:
	struct requests_t {
		size_t size;
		string_t *items;

		inline requests_t() throw() : size(0), items(NULL) { }
		inline ~requests_t() throw() { if(items) delete [] items; }

		string_t const &random() const { return items[random_U() % size]; }
	} requests;

	string_t header;

	count_t *count;

	virtual bool get_request(in_segment_t &request, in_segment_t &tag) const;
	virtual void init();
	virtual void stat(out_t &out, bool clear, bool hrr_flag) const;
	virtual void fini();

public:
	source_random_t(string_t const &, config_t const &config);
	inline ~source_random_t() throw() { delete count; }
};

namespace source_random {
config_binding_sname(source_random_t);
config_binding_value(source_random_t, requests);
config_binding_value(source_random_t, headers);
config_binding_value(source_random_t, tests);
config_binding_ctor(source_t, source_random_t);
}

source_random_t::source_random_t(string_t const &, config_t const &config) :
	requests(), header(), count(new count_t(config.tests)) {

	requests.size = 0;

	for(typeof(config.requests.ptr()) ptr = config.requests; ptr; ++ptr)
		++requests.size;

	requests.items = new string_t[requests.size];

	string_t *p = requests.items;

	for(typeof(config.requests.ptr()) ptr = config.requests; ptr; ++ptr)
		*(p++) = ptr.val().copy();

	size_t size = 0;

	for(typeof(config.headers.ptr()) ptr = config.headers; ptr; ++ptr)
		size += (ptr.val().size() + 2);

	size += 4;

	string_t::ctor_t ctor(size);

	ctor('\r')('\n');

	for(typeof(config.headers.ptr()) ptr = config.headers; ptr; ++ptr)
		ctor(ptr.val())('\r')('\n');

	header = ctor('\r')('\n');
}

bool source_random_t::get_request(in_segment_t &request, in_segment_t &) const {
	if(!count->more()) return false;

	in_segment_list_t _request;
	_request.append(requests.random());
	_request.append(header);

	request = _request;
	return true;
}

void source_random_t::init() { }

void source_random_t::stat(out_t &, bool, bool) const { }

void source_random_t::fini() { }

}}} // namespace phantom::io_benchmark::method_stream
