// This file is part of the pd::pi library.
// Copyright (C) 2012-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "pi.H"

namespace pd {

static uint32_t fnv32(char const *str, size_t size) {
	uint32_t n = 0x2000023U;

	while(size--) {
		n ^= *(str++);
		n *= 0x01000193U;
	}

	return n;
}

uint32_t pi_t::hash() const throw() {
	pi_t const *from, *to;

	if(bounds(&from, &to))
		return fnv32((char const *)from, (to - from) * sizeof(pi_t));
	else
		return fnv32((char const *)this, sizeof(pi_t));
}

template<typename uint_t>
struct pi_t::map_t::index_t {
	struct item_t {
		uint_t first, next;
	} items[0];

	inline pi_t const *lookup(
		pi_t::map_t const &map, pi_t const &key
	) const throw() {
		size_t start = key.hash() % map.count;

		for(uint_t j = items[start].first; j--; j = items[j].next)
			if(pi_t::cmp_eq(map[j].key, key))
				return &map[j].value;

		return NULL;
	}

	inline void make(pi_t::map_t const &map) throw() {
		for(uint_t i = map.count; i--;) {
			unsigned int ind = map[i].key.hash() % map.count;
			items[i].next = items[ind].first;
			items[ind].first = i + 1;
		}
	}

	inline bool verify(pi_t::map_t const &map) const throw() {
		count_t count = map.count;
		char flags[count];
		memset(flags, 0, sizeof(flags));

		for(uint_t i = 0; i < count; ++i)
			for(uint_t j = items[i].first; j--; j = items[j].next)
				if(j >= count || flags[j] || map[j].key.hash() % count != i)
					return false;

		return true;
	}
};

template<typename uint_t>
inline pi_t::map_t::index_t<uint_t> const &pi_t::map_t::index() const throw() {
	union { index_t<uint_t> const *index; pi_t::map_t::item_t const *bound; } u;
	u.bound = &items[count];
	return *u.index;
}

template<typename uint_t>
inline pi_t::map_t::index_t<uint_t> &pi_t::map_t::index() throw() {
	union { index_t<uint_t> *index; pi_t::map_t::item_t *bound; } u;
	u.bound = &items[count];
	return *u.index;
}

pi_t::_rsize_t pi_t::map_t::index_rsize(count_t count) throw() {
	if(!count)
		return 0;
	else if(count < 0xff)
		return count * sizeof(index_t<uint8_t>::item_t);
	else if(count < 0xffff)
		return count * sizeof(index_t<uint16_t>::item_t);
	else
		return count * sizeof(index_t<uint32_t>::item_t);
}

pi_t const *pi_t::map_t::lookup(pi_t const &key) const throw() {
	if(!count)
		return NULL;
	else if(count < 0xff)
		return index<uint8_t>().lookup(*this, key);
	else if(count < 0xffff)
		return index<uint16_t>().lookup(*this, key);
	else
		return index<uint32_t>().lookup(*this, key);
}

void pi_t::map_t::index_make() throw() {
	uint32_t *ptr = &items[count].key.value;
	for(_size_t _size = index_size(); _size--;) *(ptr++) = 0U;

	if(!count)
		return;
	else if(count < 0xff)
		index<uint8_t>().make(*this);
	else if(count < 0xffff)
		index<uint16_t>().make(*this);
	else
		index<uint32_t>().make(*this);
}

bool pi_t::map_t::index_verify() const throw() {
	if(!count)
		return true;
	else if(count < 0xff)
		return index<uint8_t>().verify(*this);
	else if(count < 0xffff)
		return index<uint16_t>().verify(*this);
	else
		return index<uint32_t>().verify(*this);
}

} // namespace pd
