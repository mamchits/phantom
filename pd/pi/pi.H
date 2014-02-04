// This file is part of the pd::pi library.
// Copyright (C) 2012-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/pi/vector.H>

#include <pd/base/str.H>
#include <pd/base/cmp.H>
#include <pd/base/out.H>
#include <pd/base/exception.H>

#include <stdlib.h>

#pragma GCC visibility push(default)

namespace pd {

class pi_t {
	uint32_t value;

public:
	enum type_t {
		_enum = 0,
		_int29 = 1,
		_uint64 = 2,
		_float = 3,
		_string = 4,
		_reserved = 5,
		_array = 6,
		_map = 7,
	};

	typedef uint32_t _size_t;
	typedef uint32_t _rsize_t;
	typedef uint32_t count_t;

	struct string_t;
	static string_t const null_string;

	struct array_t;
	static array_t const null_array;

	struct map_item_t;
	struct map_t;
	static map_t const null_map;

	inline type_t type() const { return (type_t)(value >> 29); }

	inline uint32_t __unsigned_value() const { return value & ~(7U << 29); }

	inline _size_t offset() const { return __unsigned_value(); }

	inline int32_t __signed_value() const {
		return (value & (1U << 28)) ? value | (7U << 29) : value & ~(7U << 29);
	}

	inline void setup(type_t type, _size_t off) {
		value = (type << 29) | (off & ~(7U << 29));
	}

	inline void setup(type_t type) { value = (type << 29); }

	inline void setup(type_t type, pi_t const *pi) { setup(type, pi - this); }

	inline pi_t() throw() : value(0) { }

	inline pi_t(type_t type, _size_t off) throw() { setup(type, off); }

	struct root_t;
	static root_t const null_root;

	struct mem_t {
		virtual void *alloc(size_t size) const = 0;
		virtual void free(void *ptr) const = 0;

		inline mem_t() throw() { }
		inline ~mem_t() throw() { }
	};

	struct err_t {
		typedef unsigned long prm_t;
		enum origin_t { _parse, _verify, _path } origin;
		str_t msg;
		prm_t aux[6];

		#define __aux(name, no) \
			prm_t &name() { return aux[no]; } \
			prm_t name() const { return aux[no]; }

		__aux(parse_lineno, 0);
		__aux(parse_pos, 1);
		__aux(parse_abspos, 2);

		__aux(verify_lev, 0);
		__aux(verify_obj, 1);
		__aux(verify_req, 2);
		__aux(verify_bound, 3);

		__aux(path_lev, 0);

		#undef __aux

		inline err_t(origin_t _origin, str_t const &_msg) throw() :
			origin(_origin), msg(_msg), aux{0, 0, 0, 0, 0, 0} { }
	};

	enum enum_t {
		_null = 0,
		_any = 1,
		_false = 2,
		_true = 3,
		_private = 0x1fff0000,
	};

	struct enum_table_t {
		virtual str_t lookup(unsigned int value) const = 0;
		virtual unsigned int lookup(str_t const &str) const = 0;

		inline enum_table_t() throw() { }
		inline ~enum_table_t() throw() { }
	};

	static enum_table_t const &enum_table_def;

	class exception_t : public pd::exception_t {
	public:
		err_t err;

		exception_t(err_t const &_err) throw();

		virtual ~exception_t() throw();
	};

	typedef root_t *(*parse_fn_t)(
		in_t::ptr_t &_ptr, mem_t const &mem, enum_table_t const &enum_table
	);

	static root_t *parse_text(
		in_t::ptr_t &_ptr, mem_t const &mem,
		enum_table_t const &enum_table = enum_table_def
	);

	static root_t *parse_app(
		in_t::ptr_t &_ptr, mem_t const &mem,
		enum_table_t const &enum_table = enum_table_def
	);

	void print(
		out_t &out, unsigned int max_inline,
		enum_table_t const &enum_table = enum_table_def
	) const;

	typedef void (*print_fn_t)(
		out_t &out, root_t const *root, enum_table_t const &enum_table
	);

	static void print_text(
		out_t &out, root_t const *root,
		enum_table_t const &enum_table = enum_table_def
	);

	static void print_app(
		out_t &out, root_t const *root,
		enum_table_t const &enum_table = enum_table_def
	);

	static root_t const *verify(
		char const *buf, size_t size,
		enum_table_t const &enum_table = enum_table_def
	);

	static root_t *replace(
		root_t const &dst_root, array_t const &path,
		pi_t const &src, mem_t const &mem
	);

	static inline root_t *create(pi_t const &pi, mem_t const &mem) {
		return replace(null_root, null_array, pi, mem);
	}

	class pro_t;
	static root_t *build(pro_t const &pro, mem_t const &mem);

	static cmp_t cmp(pi_t const &pi1, pi_t const &p2);
	static bool cmp_eq(pi_t const &pi1, pi_t const &p2);

	uint32_t hash() const throw();

	class parse_t;
	friend class parse_t;

	class print_t;
	friend class print_t;

	class verify_t;
	friend class verify_t;

	class replace_t;
	friend class replace_t;

	struct c_place_t {
		union {
			pi_t const *_pi;
			double const *__float;
			uint64_t const *__uint64;
			string_t const *__string;
			array_t const *__array;
			map_t const *__map;
			root_t const *__root;
			char const *__raw;
		};

		inline c_place_t(pi_t const *__pi) throw() : _pi(__pi) { }
		inline ~c_place_t() throw() { }
	};

	inline uint32_t __enum() const throw() {
		return value;  // _enum == 0
	}

	template<typename enum_t>
	inline enum_t __() const throw() {
		return (enum_t)(value); // _enum == 0
	}

	inline int32_t __int29() const throw() {
		return (value & (1U << 28)) ? value | (7U << 29) : value & ~(7U << 29);
	}

	inline uint64_t __uint64() const throw() {
		_size_t off = offset();
		assert(off);
		return *c_place_t(this + off).__uint64;
	}

	inline double __float() const throw() {
		_size_t off = offset();
		assert(off);
		return *c_place_t(this + off).__float;
	}

	inline string_t const &__string() const throw() {
		_size_t off = offset();
		return off ? *c_place_t(this + off).__string : null_string;
	}

	inline array_t const &__array() const throw() {
		_size_t off = offset();
		return off ? *c_place_t(this + off).__array : null_array;
	}

	inline map_t const &__map() const throw() {
		_size_t off = offset();
		return off ? *c_place_t(this + off).__map : null_map;
	}

	inline uint32_t s_enum() const throw() {
		return type() == _enum ? __enum() : 0U;
	}

	inline uint64_t s_uint() const throw() {
		return type() == _int29 ? __int29() : type() == _uint64 ? __uint64() : 0ULL;
	}

	inline int64_t s_int() const throw() {
		return type() == _int29 ? __int29() : type() == _uint64 ? __uint64() : 0LL;
	}

	inline double s_float() const throw() {
		return type() == _float ? __float() : 0.;
	}

	inline str_t s_str() const throw();
	inline pi_t const &s_ind(count_t k) const throw();

	struct place_t {
		union {
			pi_t *_pi;
			double *__float;
			uint64_t *__uint64;
			string_t *__string;
			array_t *__array;
			map_t *__map;
			root_t *__root;
			char *__raw;
		};

		inline place_t(root_t *_root) throw() : __root(_root) { }
		inline ~place_t() throw() { }
	};

	static _size_t const _number_size = 2;

	static inline _size_t __size(_rsize_t len) {
		return (len + sizeof(pi_t) - 1) / sizeof(pi_t);
	}

private:
	static int32_t const _int29_min = (15U << 28);
	static uint32_t const _int29_max = ~(15U << 28);

	inline void fix_offset(int delta) {
		type_t _type = type();

		switch(_type) {
			case _enum:
			case _int29:
				return;

			case _uint64:
			case _float:
			case _string:
			case _array:
			case _map: {
				_size_t off = offset();
				if(!off) return;
				setup(_type, off + delta);
				return;
			}
			default:;
		}
		abort();
	}

	struct obj_t : c_place_t {
		type_t _type;

		inline obj_t(type_t __type, pi_t const *__pi) throw() :
			c_place_t(__pi), _type(__type) { }

		inline ~obj_t() throw() { }

		inline operator bool() const throw() { return _pi != NULL; }

		pi_t const *bound() const throw();
	};

	inline obj_t deref() const {
		type_t _type = type();

		switch(_type) {
			case _enum:
			case _int29:
				return obj_t(_type, NULL);

			case _uint64:
			case _float:
			case _string:
			case _array:
			case _map: {
				_size_t off = offset();
				return obj_t(_type, off ? this + off : NULL);
			}
			default:;
		}
		abort();
	}

	inline bool bounds(pi_t const **from, pi_t const **to) const throw() {
		obj_t obj = deref();
		if(!obj)
			return false;

		if(from) *from = obj._pi;

		if(to) *to = obj.bound();

		return true;
	}

	pi_t(pi_t const &) = delete;
	pi_t &operator=(pi_t const &) = delete;
};

struct pi_t::string_t : public vector_t<char, count_t> {
	inline string_t() throw() : vector_t<char, count_t>() { }
	inline ~string_t() throw() { }

	inline str_t str() const throw() { return str_t(items, count); }

	static inline _size_t _size(count_t count) throw() {
		return __size(_rsize(count) + sizeof(items[0]));
	}

	inline _size_t _size() const throw() { return _size(count); }

private:
	static inline _rsize_t _rsize(count_t count) throw() {
		return sizeof(string_t) + count * sizeof(items[0]);
	}

	inline _rsize_t _rsize() const throw() { return _rsize(count); }

	friend class pi_t;
	friend class parse_t;
	friend class print_t;
	friend class verify_t;
	friend class replace_t;
};

// TMP:
inline str_t pi_t::s_str() const throw() {
	return type() == _string ? __string().str() : str_t();
}

struct pi_t::array_t : public vector_t<pi_t, count_t> {
	inline array_t() throw() : vector_t<pi_t, count_t>() { }
	inline ~array_t() throw() { }

	static inline _size_t _size(count_t count) throw() {
		return __size(sizeof(array_t) + count * sizeof(items[0]));
	}

	inline _size_t _size() const throw() { return _size(count); }

private:
	pi_t const *bound() const throw();

	friend class pi_t;
	friend class parse_t;
	friend class print_t;
	friend class verify_t;
	friend class replace_t;
};

struct pi_t::map_item_t {
	pi_t key;
	pi_t value;
};

extern pi_t const pi_null;

// TMP:
inline pi_t const &pi_t::s_ind(count_t k) const throw() {
	if(type() == _array) {
		array_t const &arr = __array();
		if(k < arr.count)
			return arr[k];
	}
	return pi_null;
}

struct pi_t::map_t : public vector_t<map_item_t, count_t> {
	typedef map_item_t item_t;

	inline map_t() throw() : vector_t<map_item_t, count_t>() { }
	inline ~map_t() throw() { }

	pi_t const *lookup(pi_t const &pi) const throw();

	static inline _size_t _size(count_t count) throw() {
		return __size(_rsize(count));
	}

	inline _size_t _size() const throw() { return _size(count); }

private:
	static _rsize_t index_rsize(count_t count) throw();

	static inline _rsize_t _rsize(count_t count) throw() {
		return sizeof(map_t) + count * sizeof(items[0]) + index_rsize(count);
	}

	static inline _size_t index_size(count_t count) throw() {
		return __size(index_rsize(count));
	}

	inline _size_t index_size() const throw() { return index_size(count); }
	inline _rsize_t _rsize() const throw() { return _rsize(count); }

	pi_t const *bound() const throw();

	void index_make() throw();
	bool index_verify() const throw();

	template<typename uint_t>
	struct index_t;

	template<typename uint_t>
	index_t<uint_t> const &index() const throw();

	template<typename uint_t>
	index_t<uint_t> &index() throw();

	friend class pi_t;
	friend class parse_t;
	friend class print_t;
	friend class verify_t;
	friend class replace_t;
};

struct pi_t::root_t {
	_size_t size;
	pi_t value;

	inline root_t() throw() : size(2) { }

// TMP:
	inline root_t(unsigned int i) throw() : size(1) {
		value.value = i & (~0U >> 3);
	}

	static inline _size_t _size() { return __size(sizeof(root_t)); }

private:
	static root_t *__new(_size_t size, mem_t const &mem) throw() {
		root_t *root = (root_t *)mem.alloc(size * sizeof(pi_t));

		if(root)
			root->size = size;

		return root;
	}

	friend class pi_t;
	friend class parse_t;
	friend class print_t;
	friend class verify_t;
	friend class replace_t;
};

} // namespace pd

#pragma GCC visibility pop
