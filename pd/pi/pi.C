// This file is part of the pd::pi library.
// Copyright (C) 2012-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "pi.H"

#include <stdlib.h>

namespace pd {

pi_t const pi_null;

pi_t::string_t const pi_t::null_string;
pi_t::array_t const pi_t::null_array;
pi_t::map_t const pi_t::null_map;
pi_t::root_t const pi_t::null_root;

cmp_t pi_t::cmp(pi_t const &pi1, pi_t const &pi2) {
	pi_t::type_t pi1t = pi1.type();
	pi_t::type_t pi2t = pi2.type();

	if(pi1t != pi2t) return pi1t - pi2t;

	switch(pi1t) {
		case _enum: return pi1.__enum() - pi2.__enum();

		case _int29: return pi1.__int29() - pi2.__int29();

		case _uint64: return pi1.__uint64() - pi2.__uint64();

		case _float: return pi1.__float() - pi2.__float();

		case _string:
			return str_t::cmp<ident_t>(pi1.__string().str(), pi2.__string().str());

		case _array: {
			array_t::c_ptr_t p1 = pi1.__array(), p2 = pi2.__array();

			for(; p1 && p2; ++p1, ++p2) {
				cmp_t _cmp = cmp(*p1, *p2);
				if(!_cmp) return _cmp;
			}

			if(p1) return 1;
			if(p2) return -1;
			return 0;
		}

		case _map: {
			map_t::c_ptr_t p1 = pi1.__map(), p2 = pi2.__map();

			for(; p1 && p2; ++p1, ++p2) {
				cmp_t _cmp = cmp(p1->key, p2->key);
				if(!_cmp) return _cmp;
				_cmp = cmp(p1->value, p2->value);
				if(!_cmp) return _cmp;
			}

			if(p1) return 1;
			if(p2) return -1;
			return 0;
		}

		default:;
	}

	abort();
}

bool pi_t::cmp_eq(pi_t const &pi1, pi_t const &pi2) {
	pi_t::type_t t = pi1.type();
	if(t != pi2.type()) return false;

	switch(t) {
		case _enum: return pi1.__enum() == pi2.__enum();

		case _int29: return pi1.__int29() == pi2.__int29();

		case _uint64: return pi1.__uint64() == pi2.__uint64();

		case _float: return pi1.__float() == pi2.__float();

		case _string:
			return str_t::cmp_eq<ident_t>(pi1.__string().str(), pi2.__string().str());

		case _array: {
			array_t const &a1 = pi1.__array(), &a2 = pi2.__array();
			count_t count = a1.count;
			if(count != a2.count)
				return false;

#if 0
			pi_t const *p1 = a1.items, *p2 = a2.items;

			for(count_t i = count; i--; ++p1, ++p2) {
				if(!cmp_eq(*p1, *p2))
					return false;
			}
#else
			if(!count)
				return true;

			pi_t const *p1 = a1.items, *p2 = a2.items;

			pi_t const *p1e = a1.bound();
			_size_t _size = p1e - p1;
			pi_t const *p2e = a2.bound();
			if(_size != (_size_t)(p2e - p2))
				return false;

			if(memcmp(p1, p2, _size * sizeof(pi_t)))
				return false;
#endif
			return true;
		}

		case _map: {
			map_t const &m1 = pi1.__map(), &m2 = pi2.__map();
			count_t count = m1.count;
			if(count != m2.count)
				return false;
#if 0
			pi_t::map_t::item_t const *p1 = m1.items, *p2 = m2.items;

			for(count_t i = count; i--; ++p1, ++p2) {
				if(!cmp_eq(p1->key, p2->key))
					return false;
				if(!cmp_eq(p1->value, p2->value))
					return false;
			}
#else
			if(!count)
				return true;

			pi_t const *p1 = &m1.items[0].key, *p2 = &m2.items[0].key;
			pi_t const *p1e = m1.bound();
			_size_t _size = p1e - p1;
			pi_t const *p2e = m2.bound();
			if(_size != (_size_t)(p2e - p2))
				return false;

			if(memcmp(p1, p2, _size * sizeof(pi_t)))
				return false;
#endif
			return true;
		}

		default:;
	}

	abort();
}

pi_t const *pi_t::array_t::bound() const throw() {
	for_each(ptr, *this) {
		obj_t pobj = ptr->deref();
		if(pobj)
			return pobj.bound();
	}

	return &items[count];
}

pi_t const *pi_t::map_t::bound() const throw() {
	for_each(ptr, *this) {
		{
			obj_t pobj = ptr->key.deref();
			if(pobj)
				return pobj.bound();
		}
		{
			obj_t pobj = ptr->value.deref();
			if(pobj)
				return pobj.bound();
		}
	}

	return &items[count].key + index_size();
}

pi_t const *pi_t::obj_t::bound() const throw() {
	switch(_type) {
		case _uint64: case _float: return _pi + _number_size;
		case _string: return _pi + __string->_size();
		case _array: return __array->bound();
		case _map: return __map->bound();

		default:
			abort();
	}
}

} // namespace pd
