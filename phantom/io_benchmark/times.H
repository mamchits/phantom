// This file is part of the phantom::io_benchmark module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/io_benchmark/mcount.H>

#include <phantom/shared.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_benchmark {

class times_t : public shared_t {
public:
	class ctor_t {
	public:
		virtual size_t size() const throw() = 0;
		virtual void fill(interval_t *_steps) const throw() = 0;

	protected:
		inline ctor_t() throw() { }
		inline ~ctor_t() throw() { }

		void *operator new(size_t) = delete;
		void operator delete(void *) = delete;
	};

private:
	struct steps_t {
		size_t size;
		interval_t *items;

		inline steps_t(ctor_t const &ctor) :
			size(ctor.size()), items(new interval_t[size]) {

			ctor.fill(items);
		}

		inline size_t index(interval_t interval) const {
			if(interval >= items[0]) return 0;
			if(interval < items[size - 1]) return(size);

			size_t left = 1, right = size - 1;

			while(left <= right) {
				size_t i = (left + right) / 2;

				if(interval >= items[i - 1])
					right = i;
				else if(interval < items[i])
					left = i + 1;
				else
					return i;
			}

			fatal("internal error");
		}

		inline ~steps_t() throw() { delete [] items; }
	};

	steps_t steps;

	class tags_t : public mcount_t::tags_t {
		size_t _size;
		string_t *items;

		virtual size_t size() const { return _size; }

		virtual void print(out_t &out, size_t idx) const {
			assert(idx < _size);
			out(items[idx]);
		}

	public:
		inline tags_t(steps_t const &steps) :
			mcount_t::tags_t(),
			_size(steps.size + 1), items(new string_t[_size]) {

			for(size_t i = 0; i < _size; ++i) {
				string_t::ctor_t ctor(64);

				if(i < _size - 1)
					ctor.print(steps.items[i] / interval::millisecond)(' ');

				ctor('-')('-');

				if(i > 0)
					ctor(' ').print(steps.items[i - 1] / interval::millisecond);

				items[i] = ctor;
			}
		}

		inline ~tags_t() throw() { delete [] items; }
	};

	tags_t tags;

	mcount_t mutable mcount;

protected:
	inline times_t(string_t const &name, ctor_t const &ctor) :
		shared_t(name), steps(ctor), tags(steps), mcount(tags) { }

	inline ~times_t() throw() { }

public:
	virtual void do_init() { mcount.init(); }
	virtual void do_run() const { }
	virtual void do_stat_print() const { mcount.print(); }
	virtual void do_fini() { }

	inline void inc(interval_t interval) { mcount.inc(steps.index(interval)); }
};

}} // namespace phantom::io_benchmark

#pragma GCC visibility pop
