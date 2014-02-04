// This file is part of the pd::pi library.
// Copyright (C) 2012-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "pi_pro.H"

namespace pd {

pi_t::_size_t pi_t::pro_t::size() const throw() {
	if(!variant) {
		pi_t const *from, *to;

		if(!pi->bounds(&from, &to))
			return 0;

		return to - from;
	}

	switch(type) {
		case _enum:
		case _int29:
			if(variant != 1) abort();

			return 0;

		case _uint64:
		case _float:
			if(variant != 1) abort();

			return _number_size;

		case _string:
			if(variant != 1) abort();

			return string_t::_size(str_val->size());

		case _array: {
			if(variant == 1) {
				_size_t size = 0;
				count_t count = array_val->count;

				for(
					pro_t const *_ptr = &array_val->items[count];
					_ptr-- > array_val->items;
				)
					size += _ptr->size();

				return pi_t::array_t::_size(count) + size;
			}
			else if(variant == 2) {
				_size_t size = 0;
				count_t count = 0;

				for(item_t const *item = items_val; item; item = item->next) {
					++count;
					size += item->value.size();
				}

				return pi_t::array_t::_size(count) + size;
			}
			else
				abort();
		}

		case _map: {
			if(variant == 1) {
				_size_t size = 0;
				count_t count = map_val->count;

				for(
					map_t::item_t const *_ptr = &map_val->items[count];
					_ptr-- > map_val->items;
				) {
					size += _ptr->value.size() + _ptr->key.size();
				}

				return pi_t::map_t::_size(count) + size;
			}
			else if(variant == 2) {
				_size_t size = 0;
				count_t count = 0;

				for(pair_t const *pair = pairs_val; pair; pair = pair->next) {
					++count;
					size += pair->value.size() + pair->key.size();
				}

				return pi_t::map_t::_size(count) + size;
			}
			else
				abort();
		}

		default:
			abort();
	}
}

void pi_t::pro_t::put(pi_t *&ref, place_t &place) const throw() {
	if(!variant) {
		pi_t const *from, *to;

		if(pi->bounds(&from, &to)) {
			_size_t size = to - from;

			memcpy(place._pi, from, size * sizeof(pi_t));

			(--ref)->setup(pi->type(), place._pi);
			place._pi += size;
		}
		else {
			(--ref)->setup(pi->type(), pi->__unsigned_value()); // FIXME
		}

		return;
	}

	switch(type) {
		case _enum: {
			if(variant != 1) abort();
			(--ref)->setup(_enum, uint_val);
		}
		break;

		case _int29: {
			if(variant != 1) abort();
			(--ref)->setup(_int29, int_val);
		}
		break;

		case _uint64: {
			if(variant != 1) abort();
			*place.__uint64 = uint_val;
			(--ref)->setup(_uint64, place._pi);
			place._pi += _number_size;
		}
		break;

		case _float: {
			if(variant != 1) abort();
			*place.__float = float_val;
			(--ref)->setup(_float, place._pi);
			place._pi += _number_size;
		}
		break;

		case _string: {
			if(variant != 1) abort();
			pi_t::string_t &__string = *place.__string;
			count_t count = str_val->size();

			__string.count = count;
			_size_t size = __string._size();

			place._pi[size - 1].value = 0U;
			memcpy(__string.items, str_val->ptr(), count);

			(--ref)->setup(_string, place._pi);
			place._pi += size;
		}
		break;

		case _array: {
			if(variant == 1) {
				pi_t::array_t &__array = *place.__array;
				count_t count = array_val->count;

				__array.count = count;
				(--ref)->setup(_array, place._pi);
				place._pi += __array._size();

				pi_t *_ref = &__array.items[count];

				for(
					pro_t const *_ptr = &array_val->items[count];
					_ptr-- > array_val->items;
				)
					_ptr->put(_ref, place);

				assert(_ref == &__array.items[0]);
			}
			else if(variant == 2) {
				pi_t::array_t &__array = *place.__array;
				count_t count = 0;

				for(item_t const *item = items_val; item; item = item->next)
					++count;

				__array.count = count;
				(--ref)->setup(_array, place._pi);
				place._pi += __array._size();

				pi_t *_ref = &__array.items[count];
				for(item_t const *item = items_val; item; item = item->next)
					item->value.put(_ref, place);

				assert(_ref == &__array.items[0]);
			}
			else
				abort();
		}
		break;

		case _map: {
			if(variant == 1) {
				pi_t::map_t &__map = *place.__map;
				count_t count = map_val->count;

				__map.count = count;
				(--ref)->setup(_map, place._pi);
				place._pi += __map._size();

				pi_t *_ref = &__map.items[count].key;

				for(
					map_t::item_t const *_ptr = &map_val->items[count];
					_ptr-- > map_val->items;
				) {
					_ptr->value.put(_ref, place);
					_ptr->key.put(_ref, place);
				}

				assert(_ref == &__map.items[0].key);

				__map.index_make();
			}
			else if(variant == 2) {
				pi_t::map_t &__map = *place.__map;
				count_t count = 0;

				for(pair_t const *pair = pairs_val; pair; pair = pair->next)
					++count;

				__map.count = count;
				(--ref)->setup(_map, place._pi);
				place._pi += __map._size();

				pi_t *_ref = &__map.items[count].key;

				for(pair_t const *pair = pairs_val; pair; pair = pair->next) {
					pair->value.put(_ref, place);
					pair->key.put(_ref, place);
				}

				assert(_ref == &__map.items[0].key);

				__map.index_make();
			}
			else
				abort();
		}
		break;

		default:
			abort();
	}
}

pi_t::root_t *pi_t::build(pro_t const &pi_pro, mem_t const &mem) {
	root_t *res_root = root_t::__new(root_t::_size() + pi_pro.size(), mem);

	place_t place = res_root;

	place._pi += res_root->_size();

	pi_t *_ref = &res_root->value + 1;
	pi_pro.put(_ref, place);
	assert(_ref == &res_root->value);

	return res_root;
}

pi_t::pro_t::item_t const *pi_t::pro_t::null_array = NULL;
pi_t::pro_t::pair_t const *pi_t::pro_t::null_map = NULL;

} // namespace pd
