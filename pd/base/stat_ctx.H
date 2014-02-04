// This file is part of the pd::base library.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/string.H>
#include <pd/base/out.H>

#pragma GCC visibility push(default)

namespace pd { namespace stat {

struct ctx_t {
	enum format_t { json, html };

	ctx_t *prev;
	out_t &out;
	format_t format;
	int variant;
	size_t size;
	size_t res_no;
	bool clear;
	str_t const tag;
	unsigned int lev;
	unsigned int count;
	bool flag;

	void off(unsigned int j = 0) const;
	void json_tag(str_t const &name, unsigned int j = 0);

	bool pre(
		str_t const &tag,
		string_t const *tags, size_t cnt,
		string_t const *stags, size_t scnt
	);

	ctx_t(
		out_t &_out, format_t _format, size_t _res_no, bool _clear,
		unsigned int _lev = 1
	);

	ctx_t(
		out_t &_out, format_t _format, size_t _res_no, bool _clear,
		str_t const &_tag, bool used, unsigned int _lev = 1
	);

	ctx_t(str_t const &tag, int _variant = 0, size_t size = 1);

	ctx_t(ctx_t const &) = delete;
	ctx_t &operator=(ctx_t const &) = delete;

	~ctx_t() throw();

	typedef ctx_t *&(*current_funct_t)();
	static current_funct_t setup(current_funct_t _current_funct) throw();
	static ctx_t *&current() throw();

	template<typename x_t>
	struct helper_t {
		static void print(
			ctx_t &ctx, str_t const &_tag,
			x_t const &val, typename x_t::res_t const &res
		);
	};

	inline ctx_t &table_root(size_t *depth = NULL) {
		if(variant < 2) {
			if(depth) *depth = 0;
			return *this;
		}

		ctx_t &res = prev->table_root(depth);
		if(depth) ++(*depth);
		return res;
	}
};

void link(string_t const &master);

extern size_t res_count;

}} // namespace pd::stat

#pragma GCC visibility pop
