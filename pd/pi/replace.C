// This file is part of the pd::pi library.
// Copyright (C) 2012-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "pi.H"

#include <stdlib.h>

namespace pd {

struct pi_t::replace_t {
	root_t const &dst_root;
	array_t const &path;
	pi_t const &src;
	mem_t const &mem;
	int32_t delta;
	root_t *res_root;

	inline replace_t(
		root_t const &_dst_root, array_t const &_path, pi_t const &_src,
		mem_t const &_mem
	) throw() :
		dst_root(_dst_root), path(_path),
		src(_src), mem(_mem), delta(0), res_root(NULL) { }

	pi_t const *find_ctx_crack(pi_t const *ctx, pi_t const *dst) throw();

	void go_path(pi_t const *dst, pi_t const *ptr, count_t idx);

	void do_replace(
		pi_t const *ctx, pi_t const *dst, pi_t const *ptr, count_t idx
	);

	inline root_t *operator()() {
		do_replace(NULL, &dst_root.value, path.items, path.count);
		return res_root;
	}

	inline void __noreturn error(char const *msg, count_t idx) {
		err_t err(err_t::_path, str_t(msg, strlen(msg)));
		err.path_lev() = path.count - idx;

		throw exception_t(err);
	}

	inline pi_t *__res(pi_t const *dst) throw() {
		return &res_root->value + (dst - &dst_root.value);
	}

	inline void copy_head(pi_t const *dst_from) throw() {
		memcpy(
			&res_root->value, &dst_root.value,
			(dst_from - &dst_root.value) * sizeof(pi_t)
		);
	}

	inline void copy_obj(
		pi_t const *dst_from, pi_t const *src_from, _size_t size
	) throw() {
		pi_t *res_from = __res(dst_from);
		memcpy(res_from, src_from, size * sizeof(pi_t));
	}

	inline void copy_tail(pi_t const *dst_to) throw() {
		pi_t *res_to = __res(dst_to) + delta;
		_size_t size = dst_root.size - (dst_to - (pi_t const *)&dst_root);
		memcpy(res_to, dst_to, size * sizeof(pi_t));
	}
};

pi_t const *pi_t::replace_t::find_ctx_crack(
	pi_t const *ctx, pi_t const *dst
) throw() {
	if(!ctx)
		return (pi_t const *)(&dst_root + 1);

	assert(ctx->offset() != 0);
	pi_t const *_ctx = ctx + ctx->offset(), *find_from, *find_to, *not_found;

	switch(ctx->type()) {
		case _array: {
			array_t const &array = *(array_t const *)_ctx;
			find_from = array.items;
			find_to = find_from + array.count;
			not_found = _ctx + array._size();
		}
		break;

		case _map: {
			map_t const &map = *(map_t const *)_ctx;
			find_from = &map.items[0].key;
			find_to = find_from + 2 * map.count;
			not_found = _ctx + map._size();
		}
		break;

		default:
			abort();
	}

	assert(dst >= find_from && dst < find_to);

	pi_t const *crack = NULL;

	for(pi_t const *ptr = dst; --ptr >= find_from;)
		if(ptr->bounds(&crack, NULL))
			return crack;

	for(pi_t const *ptr = dst; ++ptr < find_to;)
		if(ptr->bounds(NULL, &crack))
			return crack;

	return not_found;
}

void pi_t::replace_t::go_path(pi_t const *ctx, pi_t const *ptr, count_t idx) {
	pi_t const *dst_fix, *dst;

	switch(ctx->type()) {
		case _array: {
			if(ptr->type() != _int29)
				error("only small integer may index an array", idx);

			int i = ptr->__int29();
			if(i < 0)
				error("negative index", idx);

			if(!ctx->offset())
				error("index too big", idx);

			array_t const &array = ctx->__array();
			if((count_t)i >= array.count)
				error("index too big", idx);

			dst = &array.items[i];

			dst_fix = array.items;
		}
		break;

		case _map: {
			if(!ctx->offset())
				error("no such object in destination map", idx);

			map_t const &map = ctx->__map();

			dst = map.lookup(*ptr);

			if(!dst)
				error("no such object in destination map", idx);

			dst_fix = &map.items[0].key;
		}
		break;

		default: {
			if(ptr->type() == _int29 && ptr->__int29() >= 0)
				error("destination object is not a map or array", idx);
			else
				error("destination object is not a map", idx);
		}
	}

	do_replace(ctx, dst, ptr + 1, idx - 1);

	if(delta) {
		pi_t *fix_to = __res(dst);

		for(pi_t *fix = __res(dst_fix); fix < fix_to; ++fix)
			fix->fix_offset(delta);
	}

	return;
}

void pi_t::replace_t::do_replace(
	pi_t const *ctx, pi_t const *dst, pi_t const *ptr, count_t idx
) {
	if(idx)
		return go_path(dst, ptr, idx);

	pi_t const *src_from = NULL, *src_to = NULL;

	src.bounds(&src_from, &src_to);

	_size_t src_size = src_to - src_from;

	pi_t const *dst_from = NULL, *dst_to = NULL;

	delta = 0;

	if(dst->bounds(&dst_from, &dst_to)) {
		delta = src_size - (dst_to - dst_from);
	}
	else if(src_size) {
		dst_from = dst_to = find_ctx_crack(ctx, dst);
		delta = src_size;
	}

	res_root = root_t::__new(dst_root.size + delta, mem);

	if(delta || src_size) {
		copy_head(dst_from);
		if(src_size)
			copy_obj(dst_from, src_from, src_size);

		copy_tail(dst_to);
	}
	else
		copy_tail(&dst_root.value);

	if(src_size)
		__res(dst)->setup(src.type(), dst_from - dst);
	else
		__res(dst)->setup(src.type(), src.__unsigned_value());  // FIXME

	return;
}

pi_t::root_t *pi_t::replace(
	root_t const &dst_root, array_t const &path, pi_t const &src, mem_t const &mem
) {
	return replace_t(dst_root, path, src, mem)();
}

} // namespace pd
