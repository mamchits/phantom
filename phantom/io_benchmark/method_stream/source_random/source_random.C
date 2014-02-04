// This file is part of the phantom::io_benchmark::method_stream::source_random module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "../source.H"
#include "../../../module.H"

#include <pd/base/config_list.H>
#include <pd/base/random.H>
#include <pd/base/spinlock.H>
#include <pd/base/size.H>
#include <pd/base/config.H>

namespace phantom { namespace io_benchmark { namespace method_stream {

MODULE(io_benchmark_method_stream_source_random);

class source_random_t : public source_t {
	class count_t {
		spinlock_t spinlock;
		unsigned int val;

	public:
		inline count_t(unsigned int _val) : val(_val) { }

		inline bool more() {
			spinlock_guard_t _guard(spinlock);
			if(!val)
				return false;
			--val;
			return true;
		}
	};

public:
	struct config_t {
		config::list_t<string_t> requests;
		config::list_t<string_t> headers;
		sizeval_t tests;

		inline config_t() throw() :
			requests(), headers(), tests(sizeval::kilo) { }

		inline void check(in_t::ptr_t const &ptr) const {
			if(tests >= (4 * sizeval::giga))
				config::error(ptr, "tests is too big");

			if(!requests)
				config::error(ptr, "request list is empty");
		}
	};

private:
	struct requests_t : sarray1_t<string_t> {
		inline requests_t(config::list_t<string_t> const &list) :
			sarray1_t<string_t>(list) { }

		inline ~requests_t() throw() { }

		inline string_t const &random() const { return items[random_U() % size]; }
	} requests;

	string_t header;

	count_t *count;

	virtual void do_init() { }
	virtual void do_run() const { }
	virtual void do_stat_print() const { }
	virtual void do_fini() { }

	virtual bool get_request(in_segment_t &request, in_segment_t &tag) const;

public:
	source_random_t(string_t const &, config_t const &config);
	inline ~source_random_t() throw() { delete count; }
};

namespace source_random {
config_binding_sname(source_random_t);
config_binding_value(source_random_t, requests);
config_binding_value(source_random_t, headers);
config_binding_value(source_random_t, tests);
config_binding_cast(source_random_t, source_t);
config_binding_ctor(source_t, source_random_t);
}

source_random_t::source_random_t(string_t const &name, config_t const &config) :
	source_t(name), requests(config.requests), header(),
	count(new count_t(config.tests)) {

	size_t size = 0;

	for(typeof(config.headers._ptr()) ptr = config.headers; ptr; ++ptr)
		size += (ptr.val().size() + 2);

	size += 4;

	string_t::ctor_t ctor(size);

	ctor('\r')('\n');

	for(typeof(config.headers._ptr()) ptr = config.headers; ptr; ++ptr)
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

}}} // namespace phantom::io_benchmark::method_stream
