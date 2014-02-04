// This file is part of the pd::base library.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/string.H>
#include <pd/base/spinlock.H>
#include <pd/base/log.H>
#include <pd/base/vtempl.H>
#include <pd/base/stat_ctx.H>

#pragma GCC visibility push(default)

namespace pd { namespace stat {

template<int i, typename x_t>
struct item_t {
	x_t x;
	string_t const tag;

	inline item_t(string_t const &_tag) : x(), tag(_tag) { }
	inline item_t(item_t const &) = default;
	inline item_t &operator=(item_t const &) = default;
	inline ~item_t() throw() { }

	struct res_t {
		typename x_t::res_t x;
		inline res_t(x_t &_x) : x(_x) { }

		inline res_t() = default;
		inline res_t(res_t const &) = default;
		inline res_t &operator=(res_t const &) = default;
		inline ~res_t() throw() { }

		struct accum_t {
			inline accum_t(res_t &res, res_t const &_res) {
				res.x += _res.x;
			}
		};
	};

	struct printer_t {
		inline printer_t(
			ctx_t &ctx, item_t const &item, res_t const &res
		) {
			ctx_t::helper_t<typename x_t::val_t>::print(
				ctx, item.tag.str(), item.x, res.x
			);
		}
	};
};

template<typename idxs_t, typename ...args_t>
struct __items_t;

template<int ...idxs, typename ...args_t>
struct __items_t<vtempl::ints_t<idxs...>, args_t...> : item_t<idxs, args_t>... {

	template<typename ...tags_t>
	inline __items_t(tags_t const &...tags) : item_t<idxs, args_t>(tags)... { }

	__items_t(__items_t const &) = delete;
	__items_t &operator=(__items_t const &) = delete;

	struct __res_t : item_t<idxs, args_t>::res_t... {
		inline __res_t(__items_t &items) :
			item_t<idxs, args_t>::res_t(items.item_t<idxs, args_t>::x)... { }

		inline __res_t() = default;

		inline ~__res_t() { }

		inline __res_t(__res_t const &) = default;
		inline __res_t &operator=(__res_t const &) = default;

		struct accum_t : item_t<idxs, args_t>::res_t::accum_t... {
			inline accum_t(__res_t &res, __res_t const &_res) :
				item_t<idxs, args_t>::res_t::accum_t(res, _res)... { }
		};

		inline void accum(__res_t const &_res) {
			accum_t accum(*this, _res);
		}
	};

	struct printer_t : item_t<idxs, args_t>::printer_t... {
		inline printer_t(
			ctx_t &ctx, __items_t const &items, __res_t const &res
		) : item_t<idxs, args_t>::printer_t(ctx, items, res)... { }
	};
};

template<typename ...args_t>
struct items_t :
	__items_t<typename vtempl::idxs_t<sizeof ...(args_t)>::type_t, args_t...> {

	typedef typename vtempl::idxs_t<sizeof ...(args_t)>::type_t idxs_t;

	template<int i>
	typename vtempl::ind_t<i, args_t...>::type_t &item() {
		return item_t<i, typename vtempl::ind_t<i, args_t...>::type_t>::x;
	}

	typedef typename __items_t<idxs_t, args_t...>::__res_t __res_t;

	struct res_t : __res_t {
		inline res_t(items_t &items) : __res_t(items) { }
		inline res_t() = default;
		inline ~res_t() { }

		inline res_t(res_t const &) = default;
		inline res_t &operator=(res_t const &) = default;

		inline res_t &operator+=(res_t const &_res) {
			this->accum(_res);
			return *this;
		}
	};

	spinlock_t res_spinlock;
	size_t _res_count;
	res_t *ress;

	inline res_t get_res(ctx_t const &ctx) {
		size_t res_no = ctx.res_no;
		bool clear = ctx.clear;

		spinlock_guard_t guard(res_spinlock);

		res_t _res(*this);

		if(ress) {
			for(size_t i = 0; i < _res_count; ++i)
				ress[i] += _res;

			if(res_no < _res_count) {
				if(!clear)
					return ress[res_no];
				else {
					_res = ress[res_no];
					ress[res_no] = res_t();
					return _res;
				}
			}
		}

		log_error("wrong result number '%.*s'", (int)ctx.tag.size(), ctx.tag.ptr());

		return _res;
	}

protected:
	template<typename ...tags_t>
	inline items_t(tags_t const &...tags) :
		__items_t<idxs_t, args_t...>(tags...),
		res_spinlock(), _res_count(0), ress(NULL) {

		static_assert(sizeof ...(tags_t) == sizeof ...(args_t), "Wrong tags number");
	}

	inline ~items_t() throw() { delete [] ress; }

public:
	inline void init() {
		res_t *_ress = res_count ? new res_t[res_count] : NULL;
		{
			spinlock_guard_t guard(res_spinlock);
			_res_count = res_count;
			res_t *tmp = ress;
			ress = _ress;
			_ress = tmp;
		}
		delete [] _ress;
	}

	inline void __tmp_fini() { // for jemallocs mutexes
		res_t *_ress = NULL;
		{
			spinlock_guard_t guard(res_spinlock);
			_res_count = 0;
			res_t *tmp = ress;
			ress = _ress;
			_ress = tmp;
		}
		delete [] _ress;
	}

	inline void print();
};

// --------------------------------------------------------------

template<int i, typename stat_t>
struct meta_item_t {
	typedef typename stat_t::res_t res_t;

	stat_t &stat;

	inline meta_item_t(stat_t &_stat) : stat(_stat) { }
	inline meta_item_t(meta_item_t const &) = default;
	inline meta_item_t &operator=(meta_item_t const &) = delete;
	inline ~meta_item_t() throw() { }

	struct __res_t {
		res_t x;

		inline __res_t(res_t const &_x) : x(_x) { }

		inline __res_t() = delete;
		inline __res_t(__res_t const &) = default;
		inline __res_t &operator=(__res_t const &) = default;
		inline ~__res_t() throw() { }
	};

	struct init_t {
		inline init_t(meta_item_t const &meta_item) {
			meta_item.stat.init();
		}
	};

	inline res_t get_res(ctx_t const &ctx) const {
		return stat.get_res(ctx);
	}

	struct printer_t: stat_t::printer_t {
		inline printer_t(
			ctx_t &ctx, meta_item_t const &meta_item, __res_t const &__res
		) :
			stat_t::printer_t(ctx, meta_item.stat, __res.x) { }
	};
};

template<typename idxs_t, typename ...stats_t>
struct __meta_t;

template<int ...idxs, typename ...stats_t>
struct __meta_t<vtempl::ints_t<idxs...>, stats_t...> :
	meta_item_t<idxs, stats_t>... {

	inline __meta_t(stats_t &...stats) : meta_item_t<idxs, stats_t>(stats)... { }

	struct init_t : meta_item_t<idxs, stats_t>::init_t... {
		inline init_t(__meta_t const &meta) :
			meta_item_t<idxs, stats_t>::init_t(meta)... { }
	};

	inline void _init() const {
		init_t init(*this);
	}

	struct __res_t : meta_item_t<idxs, stats_t>::__res_t... {
		inline __res_t(typename meta_item_t<idxs, stats_t>::res_t const &...ress) :
			meta_item_t<idxs, stats_t>::__res_t(ress)... { }
	};

	inline __res_t get_res(ctx_t const &ctx) const {
		return __res_t(meta_item_t<idxs, stats_t>::get_res(ctx)...);
	}

	struct printer_t : meta_item_t<idxs, stats_t>::printer_t... {
		inline printer_t(
			ctx_t &ctx, __meta_t const &meta, __res_t const &res
		) :
			meta_item_t<idxs, stats_t>::printer_t(ctx, meta, res)... { }
	};

	inline void do_print(ctx_t &ctx, __res_t const &__res) const {
		printer_t printer(ctx, *this, __res);
	}

	inline void _print(ctx_t &ctx, __res_t const &__res) const {
		if(ctx.format == ctx.json) {
			do_print(ctx, __res);
		}
		else if (ctx.format == ctx.html) {
			if(ctx.variant < 1) {
				ctx_t _ctx(CSTR(""), 1); _ctx.flag = true; _print(_ctx, __res);
			}
			else if(ctx.variant == 1) {
				ctx_t _ctx(CSTR(""), 2); _print(_ctx, __res);
			}
			else {
				if(ctx.variant == 2) {
					if(!ctx.table_root().count++) {
						{ ctx_t _ctx(CSTR(""), 3); do_print(_ctx, __res); }
						{ ctx_t _ctx(CSTR(""), 4); do_print(_ctx, __res); }
						{ ctx_t _ctx(CSTR(""), 5); do_print(_ctx, __res); }
					}
				}

				do_print(ctx, __res);
			}
		}
	}
};

template<typename ...stats_t>
struct meta_t :
	__meta_t<typename vtempl::idxs_t<sizeof ...(stats_t)>::type_t, stats_t...> {

	typedef typename vtempl::idxs_t<sizeof ...(stats_t)>::type_t idxs_t;

	inline meta_t(stats_t &...stats) : __meta_t<idxs_t, stats_t...>(stats...) { }
	inline meta_t(meta_t const &) = default;
	inline meta_t &operator=(meta_t const &) = delete;

	inline void init() const {
		this->_init();
	}

	inline void print() {
		ctx_t *ctx = ctx_t::current();
		if(!ctx) return;

		this->_print(*ctx, this->get_res(*ctx));
	}
};

template<typename ...stats_t>
inline meta_t<stats_t...> meta(stats_t &...stats) {
	return meta_t<stats_t...>(stats...);
}

template<typename ...args_t>
inline void items_t<args_t...>::print() {
	meta(*this).print();
}

}} // namespace pd::stat

#pragma GCC visibility pop
