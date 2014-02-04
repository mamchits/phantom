// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/defs.H>

#include <stdint.h>

#pragma GCC visibility push(default)

namespace pd {

class sizeval_t {
	uint64_t val;

public:
	inline constexpr sizeval_t() throw() : val(0) { }
	inline constexpr sizeval_t(uint64_t _val) throw() : val(_val) { }
	inline constexpr sizeval_t(sizeval_t const &s) throw() : val(s.val) { }

	~sizeval_t() = default;

	inline sizeval_t &operator=(sizeval_t const &s) throw() {
		val = s.val;
		return *this;
	}

	inline sizeval_t &operator=(uint64_t _val) {
		val = _val;
		return *this;
	}

	inline operator uint64_t &() throw() { return val; }
	inline constexpr operator uint64_t const &() const throw() { return val; }

	static inline constexpr sizeval_t max() { return ~(uint64_t)0; }
	static inline constexpr sizeval_t min() { return 0; }
};

namespace sizeval {

constexpr sizeval_t unit((uint64_t)1);
constexpr sizeval_t kilo(unit * 1024);
constexpr sizeval_t mega(kilo * 1024);
constexpr sizeval_t giga(mega * 1024);
constexpr sizeval_t tera(giga * 1024);
constexpr sizeval_t peta(tera * 1024);
constexpr sizeval_t exa(peta * 1024);
constexpr sizeval_t unlimited(sizeval_t::max());

} // namespace sizeval

} // namespace pd

#pragma GCC visibility pop
