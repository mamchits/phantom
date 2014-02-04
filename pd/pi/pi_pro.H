// This file is part of the pd::pi library.
// Copyright (C) 2012-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/pi/pi.H>

namespace pd {

class pi_t::pro_t {
public:
	class array_t;
	class item_t;

	static item_t const *null_array;

	class map_t;
	class pair_t;

	static pair_t const *null_map;

	struct enum_t {
		uint32_t val;
		inline enum_t(uint32_t _val) throw() : val(_val) { }
		inline enum_t(enum_t const &) = default;
		inline ~enum_t() throw() { }
	};

	struct int_t {
		int32_t val;
		inline int_t(int32_t _val) throw() : val(_val) { }
		inline int_t(int_t const &) = default;
		inline ~int_t() throw() { }
	};

	struct uint_t {
		uint64_t val;
		inline uint_t(uint64_t _val) throw() : val(_val) { }
		inline uint_t(uint_t const &) = default;
		inline ~uint_t() throw() { }
	};

	struct float_t {
		double val;
		inline float_t(double _val) throw() : val(_val) { }
		inline float_t(float_t const &) = default;
		inline ~float_t() throw() { }
	};

private:
	type_t type;
	unsigned short variant;

	union {
		pi_t const *pi;
		int32_t int_val;
		uint64_t uint_val;
		double float_val;
		str_t const *str_val;
		array_t const *array_val;
		item_t const *items_val;
		map_t const *map_val;
		pair_t const *pairs_val;
	};

	static inline type_t __int_type(int64_t _val) {
		return (_val <= _int29_max && _val >= _int29_min) ? _int29 : _uint64;
	}

	static inline type_t __uint_type(uint64_t _val) {
		return _val <= _int29_max ? _int29 : _uint64;
	}

public:
	inline pro_t() throw() :
		type(_enum), variant(1), uint_val(_null) { }

	inline pro_t(pi_t const &_pi) throw() :
		type(_pi.type()), variant(0), pi(&_pi) { }

	inline pro_t(pi_t const &&) = delete;

	inline pro_t(enum_t __enum) throw() :
		type(_enum), variant(1), uint_val(__enum.val) { }

	inline pro_t(uint_t __uint) throw() :
		type(__uint_type(__uint.val)), variant(1), uint_val(__uint.val) { }

	inline pro_t(int_t __int) throw() :
		type(__int_type(__int.val)), variant(1), int_val(__int.val) { }

	inline pro_t(float_t __float) throw() :
		type(_float), variant(1), float_val(__float.val) { }

	inline pro_t(str_t const &_str) throw() :
		type(_string), variant(1), str_val(&_str) { }

	inline pro_t(str_t const &&) = delete;

	inline pro_t(array_t const &__array) throw() :
		type(_array), variant(1), array_val(&__array) { }

	inline pro_t(array_t const &&) = delete;

	inline pro_t(item_t const *_items) throw() :
		type(_array), variant(2), items_val(_items) { }

	inline pro_t(map_t const &__map) throw() :
		type(_map), variant(1), map_val(&__map) { }

	inline pro_t(map_t const &&) = delete;

	inline pro_t(pair_t const *_pairs) throw() :
		type(_map), variant(2), pairs_val(_pairs) { }

	inline operator bool() const throw() {
		return !(variant == 0 && pi->value == 0);
	}

	_size_t size() const throw();
	void put(pi_t *&ref, place_t &place) const throw();

	template<typename x_t>
	struct cvt_t;

	template<typename ...>
	class m_array_t;

	template<typename ...x_t>
	static inline m_array_t<typename cvt_t<x_t>::type_t...> array(x_t const &...x) {
		return m_array_t<typename cvt_t<x_t>::type_t...>(x...);
	}

	template<typename key_t, typename value_t>
	class m_pair_t;

	template<typename key_t, typename value_t>
	static inline m_pair_t<
		typename cvt_t<key_t>::type_t,
		typename cvt_t<value_t>::type_t
	> pair(key_t const &key, value_t const &value) {
		return m_pair_t<
			typename cvt_t<key_t>::type_t,
			typename cvt_t<value_t>::type_t
		>(key, value);
	}

	template<typename ...>
	class m_map_t;

	template<typename ...pairs_t>
	static inline m_map_t<pairs_t...> map(pairs_t const &...pairs) {
		return m_map_t<pairs_t...>(pairs...);
	}
};

struct pi_t::pro_t::array_t {
	count_t count;
	pro_t const *items;

	inline array_t(count_t _count, pro_t const *_items) throw() :
		count(_count), items(_items) { }
};

struct pi_t::pro_t::item_t {
	pro_t value;
	item_t const *next;

	inline item_t(pro_t const &_value, item_t const *_next) throw() :
		value(_value), next(_next) { }

	inline ~item_t() throw() { }
};


// -------------------------------------------------

struct pi_t::pro_t::map_t {
	count_t count;
	struct item_t {
		pro_t key;
		pro_t value;

		inline item_t() throw() : key(), value() { }

		inline item_t(pro_t const &_key, pro_t const &_value) throw() :
			key(_key), value(_value) { }

		inline item_t(item_t const &) = default;
		inline item_t &operator=(item_t const &) = default;

		inline ~item_t() throw() { }
	};

	item_t const *items;

	inline map_t(count_t _count, item_t const *_items = NULL) throw() :
		count(_count), items(_items) { }
};

struct pi_t::pro_t::pair_t {
	pro_t key;
	pro_t value;
	pair_t const *next;

	inline pair_t(
		pro_t const &_key, pro_t const &_value, pair_t const *_next = NULL
	) throw() :
		key(_key), value(_value), next(_next) { }

	inline ~pair_t() throw() { }
};

// -------------------------------------------------

template<> struct pi_t::pro_t::cvt_t<pi_t> { typedef pi_t type_t; };

template<> struct pi_t::pro_t::cvt_t<pi_t::pro_t::enum_t> { typedef enum_t type_t; };

template<> struct pi_t::pro_t::cvt_t<int> { typedef int_t type_t; };
template<> struct pi_t::pro_t::cvt_t<long> { typedef int_t type_t; };
template<> struct pi_t::pro_t::cvt_t<pi_t::pro_t::int_t> { typedef int_t type_t; };

template<> struct pi_t::pro_t::cvt_t<unsigned int> { typedef uint_t type_t; };
template<> struct pi_t::pro_t::cvt_t<unsigned long> { typedef uint_t type_t; };
template<> struct pi_t::pro_t::cvt_t<pi_t::pro_t::uint_t> { typedef uint_t type_t; };

template<> struct pi_t::pro_t::cvt_t<double> { typedef float_t type_t; };
template<> struct pi_t::pro_t::cvt_t<pi_t::pro_t::float_t> { typedef float_t type_t; };

template<> struct pi_t::pro_t::cvt_t<str_t> { typedef str_t type_t; };

template<>
struct pi_t::pro_t::cvt_t<pi_t::pro_t::array_t> {
	typedef array_t type_t;
};

template<typename ...x_t>
struct pi_t::pro_t::cvt_t<pi_t::pro_t::m_array_t<x_t...>> {
	typedef array_t type_t;
};

template<>
struct pi_t::pro_t::cvt_t<pi_t::pro_t::map_t> {
	typedef map_t type_t;
};

template<typename ...x_t>
struct pi_t::pro_t::cvt_t<pi_t::pro_t::m_map_t<x_t...>> {
	typedef map_t type_t;
};


template<typename ...x_t>
struct pi_t::pro_t::m_array_t : array_t {
	static const count_t _count = sizeof ...(x_t);

	pro_t items[_count];

	inline m_array_t(x_t const &...x) : array_t(_count, items), items{x...} { }

	inline m_array_t(m_array_t const &_m_array) :
		array_t(_count, items), items(_m_array.items) { }

	inline m_array_t &operator=(m_array_t const &_m_array) {
		if(&_m_array != this)
			items = _m_array.items;

		return *this;
	}
};

template<typename key_t, typename value_t>
struct pi_t::pro_t::m_pair_t : map_t::item_t {
	inline m_pair_t(key_t const &_key, value_t const &_value) throw() :
		map_t::item_t(_key, _value) { }
};

template<typename ...x_t, typename ...y_t>
struct pi_t::pro_t::m_map_t<typename pi_t::pro_t::m_pair_t<x_t, y_t>...> : map_t {
	static const count_t _count = sizeof ...(x_t);

	map_t::item_t items[_count];

	inline m_map_t(typename pi_t::pro_t::m_pair_t<x_t, y_t> const &...pairs) :
		map_t(_count, items), items{pairs...} { }

	inline m_map_t(m_map_t const &_m_map) :
		map_t(_count, items), items(_m_map.items) { }

	inline m_map_t &operator=(m_map_t const &_m_map) {
		if(&_m_map != this)
			items = _m_map.items;

		return *this;
	}
};

} // namaspace pd
