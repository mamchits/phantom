// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/uint128.H>

#pragma GCC visibility push(default)

namespace pd {

struct address_ipv6_t {
	uint128_t value;

	inline address_ipv6_t(uint128_t _value = 0) throw() :
		value(_value) { }

	inline operator bool() const { return value != 0; }

private:
	operator int() const; // to prevent incorrect conversions to bool
};

struct network_ipv6_t {
	address_ipv6_t prefix;
	uint8_t shift;

	inline network_ipv6_t() throw() : prefix(), shift(0) { }

	inline network_ipv6_t(address_ipv6_t const &_prefix, uint8_t _shift) throw() :
		prefix(_prefix), shift(_shift) { }

	inline ~network_ipv6_t() throw() { }

	inline bool match(address_ipv6_t const &addr) const {
		return ((addr.value ^ prefix.value) >> shift) == 0;
	}
};

} // namespace pd

#pragma GCC visibility pop
