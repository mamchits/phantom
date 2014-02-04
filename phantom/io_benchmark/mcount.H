// This file is part of the phantom::io_benchmark module.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/stat_ctx.H>
#include <pd/base/time.H>
#include <pd/base/spinlock.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_benchmark {

class mcount_t {
	spinlock_t spinlock;

	struct vec_t {
		uint64_t *items;

		inline vec_t() throw() : items(NULL) { }

		inline void init(size_t size) {
			uint64_t *_items = new uint64_t[size];
			for(size_t i = 0; i < size; ++i) _items[i] = 0;

			uint64_t *tmp = items;
			items = _items;

			delete [] tmp;
		}

		inline vec_t(size_t size) : items(NULL) { init(size); }

		inline ~vec_t() throw() { delete [] items; }

		vec_t(vec_t const &) = delete;
		vec_t &operator=(vec_t const &) = delete;

		inline vec_t(vec_t &&vec) : items(vec.items) { vec.items = NULL; }

		inline void swap(vec_t &vec) {
			uint64_t *tmp = items;
			items = vec.items;
			vec.items = tmp;
		}

		inline uint64_t &operator[](size_t idx) throw() { return items[idx]; }
		inline uint64_t const &operator[](size_t idx) const throw() { return items[idx]; }
	};

	size_t size;
	vec_t vals;
	timeval_t last_reset;

public:
	class tags_t {
	public:
		virtual size_t size() const = 0;
		virtual void print(out_t &out, size_t idx) const = 0;

	protected:
		inline tags_t() throw() { }
		inline ~tags_t() throw() { }

		void *operator new(size_t) = delete;
		void operator delete(void *) = delete;
	};

private:
	tags_t const &tags;

	struct res_t {
		size_t size;
		vec_t vals;
		interval_t interval;

		inline void swap(mcount_t &mcount) {
			timeval_t now = timeval::current();

			{
				spinlock_guard_t guard(mcount.spinlock);

				assert(size == mcount.size);

				vals.swap(mcount.vals);

				interval = now - mcount.last_reset;
				mcount.last_reset = now;
			}
		}

		inline res_t() throw() :
			size(0), vals(), interval(interval::zero) { }

		inline void init(size_t _size) { vals.init(size = _size); }

		inline res_t(size_t _size) :
			size(_size), vals(size), interval(interval::zero) { }

		res_t(res_t const &) = delete;
		res_t &operator=(res_t const &) = delete;

		inline ~res_t() throw() { }

		inline res_t &operator+=(res_t const &res) throw() {
			assert(size == res.size);

			for(size_t i = 0; i < size; ++i) vals[i] += res.vals[i];

			interval += res.interval;
		}

		inline void swap(res_t &res) {
			assert(size == res.size);

			vals.swap(res.vals);

			interval_t tmp = interval;
			interval = res.interval;
			res.interval = tmp;
		}

		inline void copy(res_t const &res) {
			assert(size == res.size);

			for(size_t i = 0; i < size; ++i) vals[i] = res.vals[i];
			interval = res.interval;
		}

		void print(stat::ctx_t &ctx, str_t const &tag, tags_t const &tags);
	};

	spinlock_t res_spinlock;
	size_t res_count;
	mcount_t::res_t *ress;

public:
	inline mcount_t(tags_t const &_tags) :
		spinlock(), size(_tags.size()), vals(size),
		last_reset(timeval::current()), tags(_tags),
		res_spinlock(), res_count(0), ress(NULL) { }

	inline ~mcount_t() throw() { delete [] ress; }

	mcount_t(mcount_t const &) = delete;
	mcount_t &operator=(mcount_t const &) = delete;

	inline void init() {
		res_t *_ress = stat::res_count ? new res_t[stat::res_count] : NULL;

		for(size_t i = 0; i < stat::res_count; ++i)
			_ress[i].init(size);

		{
			spinlock_guard_t guard(res_spinlock);
			res_count = stat::res_count;
			res_t *tmp = ress;
			ress = _ress;
			_ress = tmp;
		}

		delete [] _ress;
	}

	inline void inc(size_t idx) {
		if(idx < size) {
			spinlock_guard_t guard(spinlock);

			++vals[idx];
		}
	}

	inline void print_clear(stat::ctx_t &ctx) {
		size_t res_no = ctx.res_no;

		res_t __res(size);
		res_t _res(size);

		{
			spinlock_guard_t guard(res_spinlock);

			_res.swap(*this);

			if(ress) {
				for(size_t i = 0; i < res_count; ++i)
					ress[i] += _res;

				if(res_no < res_count)
					__res.swap(ress[res_no]);
			}
		}

		__res.print(ctx, CSTR("mcount"), tags);
	}

	inline void print_noclear(stat::ctx_t &ctx) {
		size_t res_no = ctx.res_no;

		res_t _res(size);

		{
			spinlock_guard_t guard(res_spinlock);

			_res.swap(*this);

			if(ress) {
				for(size_t i = 0; i < res_count; ++i)
					ress[i] += _res;

				if(res_no < res_count)
					_res.copy(ress[res_no]);
			}
		}

		_res.print(ctx, CSTR("mcount"), tags);
	}

	inline void print() {
		stat::ctx_t ctx(CSTR("mcount"));

		if(ctx.clear)
			print_clear(ctx);
		else
			print_noclear(ctx);
	}
};

}} // namespace phantom::io_benchmark

#pragma GCC visibility pop
