// This file is part of the pd::base library.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/op.H>
#include <pd/base/size.H>
#include <pd/base/time.H>
#include <pd/base/string.H>
#include <pd/base/spinlock.H>
#include <pd/base/stat_ctx.H>

#pragma GCC visibility push(default)

namespace pd { namespace stat {

class count_t {
	spinlock_t spinlock;
	sizeval_t val;
	timeval_t last_reset;

	inline void reset(timeval_t now) {
		val = 0;
		last_reset = now;
	}

public:
	inline count_t() throw() :
		spinlock(), val(0), last_reset(timeval::current()) { }

	inline ~count_t() throw() { }

	count_t(count_t const &) = delete;
	count_t &operator=(count_t const &) = delete;

	inline count_t &operator+=(sizeval_t _val) throw() {
		{
			spinlock_guard_t guard(spinlock);
			val += _val;
		}
		return *this;
	}

	inline count_t &operator++() throw() {
		{
			spinlock_guard_t guard(spinlock);
			++val;
		}
		return *this;
	}

	typedef count_t val_t;

	class res_t {
	public:
		sizeval_t val;
		interval_t interval;

		inline res_t(count_t &count) throw() {
			timeval_t now = timeval::current();

			spinlock_guard_t guard(count.spinlock);
			val = count.val;
			interval = now - count.last_reset;
			count.reset(now);
		}

		inline res_t() throw() {
			val = 0;
			interval = interval::zero;
		}

		inline ~res_t() throw() { }

		inline res_t(res_t const &) = default;
		inline res_t &operator=(res_t const &) = default;

		inline res_t &operator+=(res_t const &res) throw() {
			val += res.val;
			interval += res.interval;
			return *this;
		}
	};

	friend class res_t;
};

template<typename _val_t, bool linear_approx>
class mm_t {
	spinlock_t spinlock;
	_val_t val, min_val, max_val;

	typedef decltype(val * interval::zero) tsum_t;
	tsum_t tsum;

	timeval_t last_update, last_reset;

	inline void reset(timeval_t const &now) throw() {
		min_val = val;
		max_val = val;
		tsum = tsum_t(0);
		last_reset = now;
		last_update = now;
	}

	inline void disable(timeval_t const &now) {
		min_val = _val_t::max();
		max_val = _val_t::min();
		tsum = tsum_t(0);
		last_reset = now;
		last_update = now;
	}

	inline bool update(timeval_t const &now) {
		if(min_val > max_val)
			return false;

		tsum += val * (now - last_update);
		last_update = now;
		return true;
	}

	inline void setval(_val_t const &newval, timeval_t const &now) {
		if(linear_approx)
			tsum += (val + newval) * (now - last_update) / 2;
		else
			tsum += val * (now - last_update);

		val = newval;
		minimize(min_val, val);
		maximize(max_val, val);

		last_update = now;
	}

public:
	inline mm_t() throw() :
		spinlock(), val(0), min_val(0), max_val(0), tsum(0),
		last_update(timeval::current()), last_reset(last_update) { }

	inline ~mm_t() throw() { }

	mm_t(mm_t const &) = delete;
	mm_t &operator=(mm_t const &) = delete;

	inline mm_t &operator++() throw() {
		timeval_t now = timeval::current();

		{
			spinlock_guard_t guard(spinlock);
			setval(val + 1, now);
		}

		return *this;
	}

	inline mm_t &operator--() throw() {
		timeval_t now = timeval::current();

		{
			spinlock_guard_t guard(spinlock);
			setval(val - 1, now);
		}

		return *this;
	}

	inline mm_t &operator=(_val_t _val) throw() {
		timeval_t now = timeval::current();

		{
			spinlock_guard_t guard(spinlock);
			setval(_val, now);
		}

		return *this;
	}

	inline void reset() throw() {
		timeval_t now = timeval::current();
		{
			spinlock_guard_t guard(spinlock);
			reset(now);
		}
	}

	inline void disable() throw() {
		timeval_t now = timeval::current();
		{
			spinlock_guard_t guard(spinlock);
			disable(now);
		}
	}

	typedef mm_t val_t;

	class res_t {
	public:
		_val_t min_val, max_val;
		tsum_t tsum;
		interval_t interval;

		inline res_t(mm_t &mm) throw() {
			timeval_t now = timeval::current();

			spinlock_guard_t guard(mm.spinlock);
			if(mm.update(now)) {
				min_val = mm.min_val;
				max_val = mm.max_val;
				tsum = mm.tsum;
				interval = now - mm.last_reset;
				mm.reset(now);
			}
			else {
				min_val = _val_t::max();
				max_val = _val_t::min();
				tsum = tsum_t(0);
				interval = interval::zero;
			}
		}

		inline res_t() throw() {
			min_val = _val_t::max();
			max_val = _val_t::min();
			tsum = tsum_t(0);
			interval = interval::zero;
		}

		inline ~res_t() throw() { }

		inline res_t(res_t const &) = default;
		inline res_t &operator=(res_t const &) = default;

		inline res_t &operator+=(res_t const &res) {
			minimize(min_val, res.min_val);
			maximize(max_val, res.max_val);
			tsum += res.tsum;
			interval += res.interval;
			return *this;
		}
	};

	friend class res_t;
};

typedef mm_t<sizeval_t, false> mmcount_t;
typedef mm_t<interval_t, false> mminterval_t;
typedef mm_t<interval_t, true> mmage_t;

// -------------------------------------------------------------------

template <unsigned int cnt>
struct tags_t {
	string_t const tags[cnt];

	template<typename ..._tags_t>
	inline tags_t(_tags_t const &..._tags) : tags({_tags...}) {
		static_assert(sizeof ...(_tags_t) == cnt, "Wrong tags number");
	}
};

// -------------------------------------------------------------------

template <typename enum_t, unsigned int cnt>
class vcount_res_t;

template <typename enum_t, unsigned int cnt>
class vcount_t : public tags_t<cnt> {
	spinlock_t spinlock;
	sizeval_t vals[cnt];
	timeval_t last_reset;

	inline void reset(timeval_t now) throw() {
		for(unsigned int i = 0; i < cnt; ++i)
			vals[i] = 0;

		last_reset = now;
	}

public:
	typedef vcount_t val_t;
	typedef vcount_res_t<enum_t, cnt> res_t;

	template<typename ..._tags_t>
	inline vcount_t(_tags_t const &..._tags) throw() :
		tags_t<cnt>(_tags...), spinlock() {

		reset(timeval::current());
	}

	inline ~vcount_t() throw() { }

	vcount_t(vcount_t const &) = delete;
	vcount_t &operator=(vcount_t const &) = delete;

	struct item_t {
		enum_t ind;
		vcount_t &vcount;
		inline item_t(enum_t _ind, vcount_t &_vcount) :
			ind(_ind), vcount(_vcount) { }

		inline ~item_t() throw() { }
		inline item_t(item_t const &) = default;

		item_t &operator++() {
			if(ind >= 0 && ind < cnt) {
				spinlock_guard_t guard(vcount.spinlock);
				++vcount.vals[ind];
			}
			return *this;
		}

		item_t &operator+=(sizeval_t _val) {
			if(ind >= 0 && ind < cnt) {
				spinlock_guard_t guard(vcount.spinlock);
				vcount.vals[ind] += _val;
			}
			return *this;
		}
	};

	item_t operator[](enum_t ind) { return item_t(ind, *this); }

	friend class vcount_res_t<enum_t, cnt>;
};

template <typename enum_t, unsigned int cnt>
struct vcount_res_t {
	sizeval_t vals[cnt];
	interval_t interval;

	inline vcount_res_t(vcount_t<enum_t, cnt> &vcount) throw() {
		timeval_t now = timeval::current();

		spinlock_guard_t guard(vcount.spinlock);

		for(unsigned int i = 0; i < cnt; ++i)
			vals[i] = vcount.vals[i];

		interval = now - vcount.last_reset;
		vcount.reset(now);
	}

	inline vcount_res_t() throw() {
		for(unsigned int i = 0; i < cnt; ++i)
			vals[i] = 0;

		interval = interval::zero;
	}

	inline ~vcount_res_t() throw() { }

	vcount_res_t(vcount_res_t const &) = default;
	vcount_res_t &operator=(vcount_res_t const &) = default;

	inline vcount_res_t &operator+=(vcount_res_t const &res) throw() {
		for(unsigned int i = 0; i < cnt; ++i)
			vals[i] += res.vals[i];

		interval += res.interval;
		return *this;
	}
};

void vcount_print(
	ctx_t &ctx, str_t const &_tag,
	string_t const *tags, sizeval_t const *vals, size_t cnt,
	interval_t const &interval
);

template<typename enum_t, unsigned int cnt>
struct ctx_t::helper_t<vcount_t<enum_t, cnt>> {
	typedef vcount_t<enum_t, cnt> val_t;

	static inline void print(
		ctx_t &ctx, str_t const &_tag,
		val_t const &vcount, typename val_t::res_t const &vcount_res
	) {
		vcount_print(
			ctx, _tag, vcount.tags, vcount_res.vals, cnt, vcount_res.interval
		);
	}
};

// -------------------------------------------------------------------

template <typename enum_t, unsigned int cnt>
class tstate_res_t;

template <typename enum_t, unsigned int cnt>
class tstate_t : public tags_t<cnt> {
	spinlock_t spinlock;
	enum_t val;
	interval_t times[cnt];
	timeval_t last_update;

	inline void update(timeval_t const &now) {
		times[val] += (now - last_update);
		last_update = now;
	}

	inline void reset() throw() {
		for(unsigned int i = 0; i < cnt; ++i)
			times[i] = interval::zero;
	}

public:
	typedef tstate_t val_t;
	typedef tstate_res_t<enum_t, cnt> res_t;

	template<typename ..._tags_t>
	inline tstate_t(_tags_t const &..._tags) throw() :
		tags_t<cnt>(_tags...),
		spinlock(), val((enum_t)0), last_update(timeval::current()) {

		for(unsigned int i = 0; i < cnt; ++i)
			times[i] = interval::zero;
	}

	inline ~tstate_t() throw() { }

	tstate_t(tstate_t const &) = delete;
	tstate_t &operator=(tstate_t const &) = delete;

	inline enum_t set(enum_t _val) {
		if(_val >= 0 && _val < cnt) {
			timeval_t now = timeval::current();

			spinlock_guard_t guard(spinlock);
			update(now);
			enum_t prev = val;
			val = _val;
			return prev;
		}

		return val;
	}

	friend class tstate_res_t<enum_t, cnt>;
};

template <typename enum_t, unsigned int cnt>
struct tstate_res_t {
	interval_t times[cnt];

	inline tstate_res_t(tstate_t<enum_t, cnt> &tstate) throw() {
		timeval_t now = timeval::current();

		spinlock_guard_t guard(tstate.spinlock);
		tstate.update(now);

		for(unsigned int i = 0; i < cnt; ++i)
			times[i] = tstate.times[i];

		tstate.reset();
	}

	inline tstate_res_t() throw() {
		for(unsigned int i = 0; i < cnt; ++i)
			times[i] = interval::zero;
	}

	inline ~tstate_res_t() throw() { }

	inline tstate_res_t(tstate_res_t const &) = default;
	inline tstate_res_t &operator=(tstate_res_t const &) = default;

	inline tstate_res_t &operator+=(tstate_res_t const &res) throw() {
		for(unsigned int i = 0; i < cnt; ++i)
			times[i] += res.times[i];

		return *this;
	}
};

void tstate_print(
	ctx_t &ctx, str_t const &_tag,
	string_t const *tags, interval_t const *intervals, size_t cnt
);

template<typename enum_t, unsigned int cnt>
struct ctx_t::helper_t<tstate_t<enum_t, cnt>> {
	typedef tstate_t<enum_t, cnt> val_t;

	static inline void print(
		ctx_t &ctx, str_t const &_tag,
		val_t const &tstate, typename val_t::res_t const &tstate_res
	) {
		tstate_print(ctx, _tag, tstate.tags, tstate_res.times, cnt);
	}
};

}} // namespace pd::stat

#pragma GCC visibility pop
